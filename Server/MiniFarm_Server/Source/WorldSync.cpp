#include "WorldSync.h"
#include "GameWorld.h"
#include "../../../Shared/Protocol.h"

// main.cpp에 정의된 월드를 사용한다.
extern GameWorld g_world;

// main.cpp에 구현한 송신 함수를 사용한다.
bool SendToClient(int sessionId, const char* packet);

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
	std::vector<FieldState> fields;

	// 같은 시점의 밭 상태를 복사한다. 송신 중에는 월드 잠금을 잡지 않는다.
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

	for (const FieldState& field : fields) {
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

		if (!SendToClient(sessionId, (const char*)&update)) {
			return false;
		}
	}
	return true;
}
