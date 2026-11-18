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
	// 플레이어 초기 상태·이동·장비·퇴장 전송의 순서를 맞춘다.
	// 이 잠금 안에서도 월드 잠금은 복사·준비 완료 표시 중에만 잡는다.
	std::mutex s_playerSyncMutex;

	struct PlayerSyncState {
		S2C_MOVE_OBJECT_PACKET move{};
		S2C_EQUIP_OBJECT_PACKET equip{};
	};

	PlayerSyncState MakePlayerState(const PlayerState& player)
	{
		PlayerSyncState state;
		const PlayerPosition& pos = player.GetPosition();
		state.move.header.size = (unsigned char)sizeof(state.move);
		state.move.header.type = S2C_MOVE_OBJECT;
		state.move.objectId = player.GetId();
		state.move.x = pos.x;
		state.move.y = pos.y;
		state.move.z = pos.z;
		state.move.yaw = player.GetYaw();
		state.move.isMoving = player.IsMoving() ? 1 : 0;
		state.move.isAirborne = player.IsAirborne() ? 1 : 0;
		state.equip.header.size = (unsigned char)sizeof(state.equip);
		state.equip.header.type = S2C_EQUIP_OBJECT;
		state.equip.objectId = player.GetId();
		state.equip.itemId = static_cast<std::int32_t>(player.GetEquippedTool());
		return state;
	}

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
		const bool sent = SendToClient(sessionId, (const char*)&update);
		return sent; // 초기·변경 상태의 전송 결과를 디버거에서 확인한다.
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

	// 까마귀는 위치가 바뀔 수 있으므로 월드 잠금을 잡고 복사한 뒤 보낸다
	std::vector<CrowState> crows;
	g_world.Lock();
	crows = g_world.GetCrows();
	g_world.Unlock();

	for (const CrowState& crow : crows) {
		S2C_ADD_OBJECT_PACKET add{};
		add.header.size = (unsigned char)sizeof(add);
		add.header.type = S2C_ADD_OBJECT;
		add.objectId = crow.id;
		add.objectType = 3; // 까마귀
		add.pos[0] = crow.pos.x; add.pos[1] = crow.pos.y; add.pos[2] = crow.pos.z;
		add.scale[0] = 1.f;      add.scale[1] = 1.f;      add.scale[2] = 1.f;

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

	// 각 접속자의 실제 전송값을 남겨 초기화 도중 바뀐 상태도 다음 갱신에서 찾는다.
	if (GameConfig::ServerFarmingEnabled)
		s_sentFields[sessionId] = std::move(fields);
	return true;
}

bool SendPlayerStates(int sessionId)
{
	std::lock_guard<std::mutex> syncLock(s_playerSyncMutex);
	std::vector<PlayerSyncState> states;
	PlayerSyncState joining;

	g_world.Lock();
	PlayerState* player = g_world.GetPlayer(sessionId);
	if (player == nullptr || player->IsFieldSyncReady())
	{
		g_world.Unlock();
		return false;
	}

	joining = MakePlayerState(*player);
	for (int playerId : g_world.GetFieldSyncPlayers())
	{
		const PlayerState* other = g_world.GetPlayer(playerId);
		if (other != nullptr)
			states.push_back(MakePlayerState(*other));
	}
	g_world.Unlock();

	// 후접속자는 위치 패킷으로 상대를 생성한 뒤 같은 ID의 장비를 적용한다.
	for (const PlayerSyncState& state : states)
	{
		if (!SendToClient(sessionId, (const char*)&state.move))
			return false;
		const bool sent = SendToClient(sessionId, (const char*)&state.equip);
		if (!sent)
			return false;
	}

	// 객체·밭·플레이어 초기 전송을 모두 마친 뒤 실시간 수신 대상에 넣는다.
	g_world.Lock();
	player = g_world.GetPlayer(sessionId);
	if (player != nullptr)
		player->SetFieldSyncReady(true);
	const bool ready = player != nullptr;
	g_world.Unlock();
	if (!ready)
		return false;

	// 기존 접속자에게도 새 플레이어를 알린다. 다시 움직일 때까지 기다리지 않는다.
	for (const PlayerSyncState& state : states)
	{
		const int receiverId = state.move.objectId;
		if (!SendToClient(receiverId, (const char*)&joining.move)
			|| !SendToClient(receiverId, (const char*)&joining.equip))
		{
			printf("[새 플레이어 전송 실패] 대상 ID=%d\n", receiverId);
		}
	}
	return true;
}

void SendChangedWalletBalances()
{
	S2C_WALLET_BALANCE_PACKET update{};
	update.header.size = static_cast<unsigned char>(sizeof(update));
	update.header.type = S2C_WALLET_BALANCE;

	std::vector<int> receivers;
	g_world.Lock();
	update.balance = g_world.GetWallet().GetMoney();
	for (int sessionId : g_world.GetFieldSyncPlayers())
	{
		if (g_world.GetPlayer(sessionId)->GetLastSentWalletBalance() != update.balance)
			receivers.push_back(sessionId);
	}
	g_world.Unlock();

	// The update thread sends both initial and changed balances in order.
	for (int sessionId : receivers)
	{
		if (!SendToClient(sessionId, reinterpret_cast<const char*>(&update)))
			continue;

		g_world.Lock();
		PlayerState* player = g_world.GetPlayer(sessionId);
		if (player != nullptr)
			player->SetLastSentWalletBalance(update.balance);
		g_world.Unlock();
#ifdef _DEBUG
		printf("[Wallet sent] playerId=%d, balance=%d\n", sessionId, static_cast<int>(update.balance));
#endif
	}
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

			const bool sent = SendFieldState(sessionId, fields[i]);
			if (!sent)
				break;

#ifdef _DEBUG
			// 성장 시간 갱신마다 출력하지 않고 종류·단계가 바뀐 경우만 기록한다.
			if (i >= previous.size() || previous[i].crop.id != fields[i].crop.id
				|| previous[i].crop.level != fields[i].crop.level) {
				printf("[Field update sent] receiverId=%d, objectId=%d, cropId=%d, level=%d\n",
					sessionId, fields[i].objectId, static_cast<int>(fields[i].crop.id), fields[i].crop.level);
			}
#endif

			// 성공한 전송만 기록한다. 성장 시간만 바뀐 경우도 다음 비교에 포함된다.
			if (i < previous.size())
				previous[i] = fields[i];
			else
				previous.push_back(fields[i]);
		}
	}
}

void SendPlayerMove(int sessionId)
{
	std::lock_guard<std::mutex> syncLock(s_playerSyncMutex);
	S2C_MOVE_OBJECT_PACKET update{};
	update.header.size = (unsigned char)sizeof(update);
	update.header.type = S2C_MOVE_OBJECT;

	std::vector<int> receivers;

	g_world.Lock();

	PlayerState* player = g_world.GetPlayer(sessionId);

	if (player == nullptr)
	{
		g_world.Unlock();
		return;
	}

	const PlayerPosition& pos = player->GetPosition();

	update.objectId = player->GetId();
	update.x = pos.x;
	update.y = pos.y;
	update.z = pos.z;
	update.yaw = player->GetYaw();
	update.isMoving = player->IsMoving() ? 1 : 0;
	update.isAirborne = player->IsAirborne() ? 1 : 0;

	// 현재 이 목록은 초기 객체·밭 상태 전송까지 끝난 접속자들이다.
	receivers = g_world.GetFieldSyncPlayers();

	g_world.Unlock();

	// 월드 잠금을 해제한 뒤 송신한다.
	for (int receiverId : receivers)
	{
		if (receiverId == sessionId)
			continue;

		if (!SendToClient(receiverId, (const char*)&update))
		{
			printf("[이동 전송 실패] 대상 ID=%d\n", receiverId);
		}
	}
}

void SendPlayerEquip(int sessionId)
{
	std::lock_guard<std::mutex> syncLock(s_playerSyncMutex);
	S2C_EQUIP_OBJECT_PACKET update{};
	update.header.size = (unsigned char)sizeof(update);
	update.header.type = S2C_EQUIP_OBJECT;

	std::vector<int> receivers;

	g_world.Lock();
	PlayerState* player = g_world.GetPlayer(sessionId);
	if (player == nullptr)
	{
		g_world.Unlock();
		return;
	}

	update.objectId = player->GetId();
	update.itemId = static_cast<std::int32_t>(player->GetEquippedTool());
	receivers = g_world.GetFieldSyncPlayers();
	g_world.Unlock();

	// 초기 동기화가 끝난 상대에게만 월드 잠금 없이 전송한다.
	for (int receiverId : receivers)
	{
		if (receiverId == sessionId)
			continue;

		const bool sent = SendToClient(receiverId, (const char*)&update);
		if (!sent)
		{
			printf("[장비 전송 실패] 대상 ID=%d\n", receiverId);
		}
	}
}

void BroadcastRemoveObject(int objectId)
{
	std::lock_guard<std::mutex> syncLock(s_playerSyncMutex);
	S2C_REMOVE_OBJECT_PACKET remove{};
	remove.header.size = (unsigned char)sizeof(remove);
	remove.header.type = S2C_REMOVE_OBJECT;
	remove.objectId = objectId;

	g_world.Lock();
	const std::vector<int> receivers = g_world.GetFieldSyncPlayers();
	g_world.Unlock();

	// 로그인 응답·초기 상태를 받기 전인 접속자에게는 보내지 않는다.
	for (int receiverId : receivers)
	{
		if (!SendToClient(receiverId, (const char*)&remove))
			printf("[객체 삭제 전송 실패] 대상 ID=%d\n", receiverId);
	}
}
