#include "WorldSync.h"
#include "GameWorld.h"
#include "../../../Shared/Protocol.h"
#include "../../../Common/GameConfig.h"
#include <algorithm>
#include <mutex>
#include <unordered_map>

// main.cpp에 정의된 월드를 사용한다.
extern GameWorld g_world;

// main.cpp에 구현한 송신 함수를 사용한다.
bool SendToClient(int sessionId, const char* packet);

namespace {
	// 초기 상태와 변경 상태의 전송 순서를 맞춘다. 월드 잠금과는 별도로 관리한다.
	std::mutex s_fieldSyncMutex;
	std::unordered_map<int, std::vector<FieldState>> s_sentFields;

	bool CopyFieldStates(std::vector<FieldState>& fields)
	{
		g_world.Lock();
		fields.reserve(g_world.GetFieldCount());
		for (const WorldObject& obj : g_world.GetObjects()) {
			if (obj.type != WorldObjectType::Field)
				continue;

			const FieldState* field = g_world.GetField(obj.objectId);
			if (field == nullptr) {
				g_world.Unlock();
				return false;
			}
			fields.push_back(*field);
		}
		g_world.Unlock();
		return true;
	}

	bool SendFieldState(int sessionId, const FieldState& field)
	{
		const CropState& crop = field.crop;
		S2C_FIELD_UPDATE_PACKET update{};
		update.header.size = (unsigned char)sizeof(update);
		update.header.type = S2C_FIELD_UPDATE;
		update.objectId = field.objectId;
		update.cropId = static_cast<std::int32_t>(crop.id);
		update.level = crop.level;
		update.water = crop.water;
		update.growTime = crop.growTime;
		update.requiredGrowTime = crop.requiredGrowTime;
		return SendToClient(sessionId, (const char*)&update);
	}

	bool SameFieldState(const FieldState& a, const FieldState& b)
	{
		return a.objectId == b.objectId && a.crop.id == b.crop.id
			&& a.crop.level == b.crop.level && a.crop.water == b.crop.water
			&& a.crop.growTime == b.crop.growTime
			&& a.crop.requiredGrowTime == b.crop.requiredGrowTime;
	}
}

// 객체 목록은 서버 시작 때 읽은 뒤 바뀌지 않으므로 월드 잠금 없이 읽는다.
bool SendWorldObjects(int sessionId)
{
	for (const WorldObject& obj : g_world.GetObjects()) {
		S2C_ADD_OBJECT_PACKET add{};
		add.header.size = (unsigned char)sizeof(add);
		add.header.type = S2C_ADD_OBJECT;
		add.objectId = obj.objectId;
		add.objectType = (unsigned char)obj.type; // 0 밭, 1 집, 2 보트 (WorldObjectType 순서)

		add.pos[0] = obj.pos.x;     add.pos[1] = obj.pos.y;     add.pos[2] = obj.pos.z;
		add.rot[0] = obj.rot.x;     add.rot[1] = obj.rot.y;     add.rot[2] = obj.rot.z;
		add.scale[0] = obj.scale.x; add.scale[1] = obj.scale.y; add.scale[2] = obj.scale.z;

		if (!SendToClient(sessionId, (const char*)&add)) {
			return false;
		}
	}
	return true;
}

bool SendFieldStates(int sessionId)
{
	std::lock_guard<std::mutex> syncLock(s_fieldSyncMutex);
	std::vector<FieldState> fields;
	if (!CopyFieldStates(fields))
		return false;

	for (const FieldState& field : fields) {
		if (!SendFieldState(sessionId, field)) {
			return false;
		}
	}

	g_world.Lock();
	PlayerState* player = g_world.GetPlayer(sessionId);
	if (player != nullptr)
		player->SetFieldSyncReady(true);
	const bool ready = player != nullptr;
	g_world.Unlock();
	if (!ready)
		return false;

	// 각 접속자의 실제 전송값을 남겨 초기화 도중 바뀐 상태도 다음 갱신에서 찾는다.
	if (GameConfig::ServerFarmingEnabled)
		s_sentFields[sessionId] = std::move(fields);
	return true;
}

void SendChangedFieldStates()
{
	// 로컬 농사를 유지하는 동안 서버의 반복 갱신으로 클라이언트 상태를 덮지 않는다.
	if (!GameConfig::ServerFarmingEnabled)
		return;

	std::lock_guard<std::mutex> syncLock(s_fieldSyncMutex);
	std::vector<FieldState> fields;
	if (!CopyFieldStates(fields))
		return;

	g_world.Lock();
	const std::vector<int> players = g_world.GetFieldSyncPlayers();
	g_world.Unlock();

	// 퇴장한 플레이어의 전송 기록을 정리한다.
	for (auto it = s_sentFields.begin(); it != s_sentFields.end();) {
		if (std::find(players.begin(), players.end(), it->first) == players.end())
			it = s_sentFields.erase(it);
		else
			++it;
	}

	for (int sessionId : players) {
		std::vector<FieldState>& previous = s_sentFields[sessionId];
		for (std::size_t i = 0; i < fields.size(); ++i) {
			if (i < previous.size() && SameFieldState(previous[i], fields[i]))
				continue;

			if (!SendFieldState(sessionId, fields[i]))
				break;

			// 성공한 전송만 기록한다. 성장 시간만 바뀐 경우도 다음 비교에 포함된다.
			if (i < previous.size())
				previous[i] = fields[i];
			else
				previous.push_back(fields[i]);
		}
	}
}
