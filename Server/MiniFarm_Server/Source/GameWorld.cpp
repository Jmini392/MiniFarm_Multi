#include "PCH.h"
#include "GameWorld.h"
#include "../../../Common/CropLogic.h"
#include <rapidjson/document.h>

// "pos": [x, y, z] 같은 숫자 3개짜리 배열을 읽는다
static bool ReadVec3(const rapidjson::Value& obj, const char* name, Vec3f& out) {
	if (!obj.HasMember(name)) {
		return false;
	}
	const rapidjson::Value& v = obj[name];
	if (!v.IsArray() || v.Size() != 3) {
		return false;
	}

	for (rapidjson::SizeType i = 0; i < 3; i++) {
		if (!v[i].IsNumber()) {
			return false;
		}
	}

	out.x = v[0].GetFloat();
	out.y = v[1].GetFloat();
	out.z = v[2].GetFloat();
	
	return true;
}

static bool ParseType(const char* name, WorldObjectType& type) {
	if (strcmp(name, "Field") == 0) { type = WorldObjectType::Field; return true; }
	if (strcmp(name, "House") == 0) { type = WorldObjectType::House; return true; }
	if (strcmp(name, "Boat") == 0) { type = WorldObjectType::Boat; return true; }
	
	return false;
}

static bool LoadWorldObjects(const char* path, std::vector<WorldObject>& out, std::string& error) {
	// 파일 전체를 읽는다
	std::ifstream file(path, std::ios::binary);

	if (!file) {
		error = std::string("파일을 열 수 없음: ") + path;
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string json = buffer.str();

	rapidjson::Document doc;
	doc.Parse(json.c_str());
	if (doc.HasParseError()) {
		error = "JSON 문법 오류 (위치 " + std::to_string(doc.GetErrorOffset()) + ")";
		return false;
	}
	if (!doc.IsObject() || !doc.HasMember("objects") || !doc["objects"].IsArray()) {
		error = "\"objects\" 배열이 없음";
		return false;
	}

	// 객체를 하나씩 읽는다. 하나라도 이상하면 전체를 실패로 처리한다
	std::vector<WorldObject> result;
	const rapidjson::Value& arr = doc["objects"];
	for (rapidjson::SizeType i = 0; i < arr.Size(); i++) {
		const rapidjson::Value& item = arr[i];
		std::string where = "objects[" + std::to_string(i) + "]: ";

		WorldObject obj;
		obj.objectId = WORLD_OBJECT_ID_BASE + (int)i;

		if (!item.IsObject() || !item.HasMember("type") || !item["type"].IsString()) {
			error = where + "type이 없음";
			return false;
		}
		if (!ParseType(item["type"].GetString(), obj.type)) {
			error = where + "알 수 없는 type: " + item["type"].GetString();
			return false;
		}
		if (!ReadVec3(item, "pos", obj.pos) ||
			!ReadVec3(item, "rot", obj.rot) ||
			!ReadVec3(item, "scale", obj.scale)) {
			error = where + "pos, rot, scale은 숫자 3개짜리 배열이어야 함";
			return false;
		}
		result.push_back(obj);
	}

	out = std::move(result);
	return true;
}

bool GameWorld::Init(const char* cropCsvPath) {
	// 작물 데이터 읽기
	std::string error;
	if (!m_cropTable.Load(cropCsvPath, error)) {
		printf("[월드] 작물 데이터 로딩 실패: %s\n", error.c_str());
		return false;
	}
	return true;
}

bool GameWorld::LoadObjects(const char* objectsJsonPath) {
	std::vector<WorldObject> objects;
	std::string error;

	if (!LoadWorldObjects(objectsJsonPath, objects, error)) {
		printf("[월드] 배치 데이터 로딩 실패: %s\n", error.c_str());
		return false;
	}

	// 밭은 작물 상태를 가지므로 FieldState를 함께 등록한다
	for (const WorldObject& obj : objects) {
		if (obj.type == WorldObjectType::Field && !AddField(obj.objectId)) {
			printf("[월드] 밭 등록 실패: 객체 ID %d\n", obj.objectId);
			return false;
		}
	}

	m_objects = std::move(objects);
	
	return true;
}

void GameWorld::AddPlayer(int sessionId) {
	m_players.try_emplace(sessionId, sessionId);
}

void GameWorld::RemovePlayer(int sessionId) {
	m_players.erase(sessionId);
}

PlayerState* GameWorld::GetPlayer(int sessionId) {
	auto it = m_players.find(sessionId);

	if (it == m_players.end())
		return nullptr;

	return &it->second;
}

InventoryCore* GameWorld::GetInventory(int sessionId) {
	PlayerState* player = GetPlayer(sessionId);

	if (player == nullptr)
		return nullptr;

	return &player->GetInventory();
}

bool GameWorld::AddField(int objectId) {
	if (objectId < 0) {
		return false;
	}

	FieldState field;
	field.objectId = objectId;
	return m_fields.emplace(objectId, field).second;
}

FieldState* GameWorld::GetField(int objectId) {
	auto it = m_fields.find(objectId);
	if (it == m_fields.end()) {
		return nullptr;
	}

	return &it->second;
}

const char* GetFieldInteractStatusName(FieldInteractStatus status) {
    switch (status) {
    case FieldInteractStatus::Success: return "Success";
    case FieldInteractStatus::PlayerNotReady: return "PlayerNotReady";
    case FieldInteractStatus::ServerFarmingDisabled: return "ServerFarmingDisabled";
    case FieldInteractStatus::UnknownField: return "UnknownField";
    case FieldInteractStatus::TooFar: return "TooFar";
    case FieldInteractStatus::UnsupportedTool: return "UnsupportedTool";
    case FieldInteractStatus::NotApplicable: return "NotApplicable";
    case FieldInteractStatus::NoSeed: return "NoSeed";
    case FieldInteractStatus::InvalidCropData: return "InvalidCropData";
    case FieldInteractStatus::NotMature: return "NotMature";
    case FieldInteractStatus::InventoryLimitExceeded: return "InventoryLimitExceeded";
    case FieldInteractStatus::InvalidCropState: return "InvalidCropState";
    default: return "Unknown";
    }
}

FieldInteractResult GameWorld::InteractField(int sessionId, int objectId) {
    // 호출부의 월드 잠금을 유지한 채 검사와 두 상태 변경을 함께 처리한다.
    FieldInteractResult result;
    PlayerState* player = GetPlayer(sessionId);
    if (player == nullptr || !player->IsFieldSyncReady())
        return result;

    result.equippedTool = player->GetEquippedTool();
    if (!GameConfig::ServerFarmingEnabled) {
        result.status = FieldInteractStatus::ServerFarmingDisabled;
        return result;
    }

    FieldState* field = GetField(objectId);
    const WorldObject* object = nullptr;
    
	for (const WorldObject& candidate : m_objects) {
        if (candidate.objectId == objectId && candidate.type == WorldObjectType::Field) {
            object = &candidate;
            break;
        }
    }

    if (field == nullptr || object == nullptr) {
        result.status = FieldInteractStatus::UnknownField;
        return result;
    }

    result.before = field->crop;
    result.after = field->crop;
    InventoryCore& inventory = player->GetInventory();
    const bool isSeed = result.equippedTool == EItemID::SeedCarrot
        || result.equippedTool == EItemID::SeedCabbage;
    
	if (isSeed) {
        result.seedCountBefore = inventory.GetItemCount(result.equippedTool);
        result.seedCountAfter = result.seedCountBefore;
    }

    // 배치 원본의 X만 반전하면 플레이어 좌표와 같아지며, 클라이언트와 같은 3차원 거리 2.5로 검사한다.
    const PlayerPosition& pos = player->GetPosition();
    const double dx = static_cast<double>(pos.x) + object->pos.x;
    const double dy = static_cast<double>(pos.y) - object->pos.y;
    const double dz = static_cast<double>(pos.z) - object->pos.z;
    result.distanceSquared = dx * dx + dy * dy + dz * dz;
    constexpr double INTERACT_DISTANCE = 2.5;
    if (!std::isfinite(result.distanceSquared)
        || result.distanceSquared > INTERACT_DISTANCE * INTERACT_DISTANCE) {
        result.status = FieldInteractStatus::TooFar;
        return result;
    }

    CropLogic::ActionResult action;
    if (result.equippedTool == EItemID::Hoe) {
        action = CropLogic::Till(field->crop);
    }
    else if (isSeed) {
        const EItemID cropId = result.equippedTool == EItemID::SeedCarrot
            ? EItemID::Carrot : EItemID::Cabbage;
        if (m_cropTable.GetCrop(cropId) == nullptr) {
            result.status = FieldInteractStatus::InvalidCropData;
            return result;
        }
        // Plant가 씨앗 1개를 차감한다. 여기서 다시 차감하지 않는다.
        action = CropLogic::Plant(field->crop, inventory, result.equippedTool);
        result.seedCountAfter = inventory.GetItemCount(result.equippedTool);
    }
    else if (result.equippedTool == EItemID::WateringCan) {
        action = CropLogic::Water(field->crop);
    }
    else if (result.equippedTool == EItemID::Sickle) {
        const CropData* data = m_cropTable.GetCrop(field->crop.id);
        const CropData missingData{};
        action = CropLogic::Harvest(field->crop, inventory, data ? *data : missingData);
    }
    else {
        result.status = FieldInteractStatus::UnsupportedTool;
        return result;
    }

    result.after = field->crop;
    switch (action) {
    case CropLogic::ActionResult::Success:
        result.status = FieldInteractStatus::Success;
        break;
    case CropLogic::ActionResult::NotApplicable:
        result.status = FieldInteractStatus::NotApplicable;
        break;
    case CropLogic::ActionResult::NoSeed:
        result.status = FieldInteractStatus::NoSeed;
        break;
    case CropLogic::ActionResult::NotMature:
        result.status = FieldInteractStatus::NotMature;
        break;
    case CropLogic::ActionResult::InventoryLimitExceeded:
        result.status = FieldInteractStatus::InventoryLimitExceeded;
        break;
    case CropLogic::ActionResult::InvalidState:
        result.status = FieldInteractStatus::InvalidCropState;
        break;
    case CropLogic::ActionResult::InvalidData:
        result.status = FieldInteractStatus::InvalidCropData;
        break;
    }
    return result;
}

std::vector<int> GameWorld::GetFieldSyncPlayers() const {
	std::vector<int> players;
	for (const auto& pair : m_players) {
		if (pair.second.IsFieldSyncReady())
			players.push_back(pair.first);
	}
	return players;
}

std::vector<Vec3f> GameWorld::GetPlayerPositions() const {
	std::vector<Vec3f> positions;
	positions.reserve(m_players.size());
	for (const auto& pair : m_players) {
		const PlayerPosition& pos = pair.second.GetPosition();
		positions.push_back({ pos.x, pos.y, pos.z });
	}
	return positions;
}

bool GameWorld::HasCrop(int fieldId) const {
	auto it = m_fields.find(fieldId);
	if (it == m_fields.end()) return false;
	const EItemID id = it->second.crop.id;
	return id == EItemID::Carrot || id == EItemID::Cabbage;
}

void GameWorld::Update(int dtMs) {
	m_timeSystem.Update(dtMs);
	m_crowSystem.Update(dtMs, *this);

	// 모든 밭의 작물을 지난 시간만큼 성장시킨다
	for (auto& pair : m_fields) {
		CropState& crop = pair.second.crop;
		const CropData* data = m_cropTable.GetCrop(crop.id);
		if (data == nullptr) {
			continue; // 빈 밭이거나 데이터가 없는 작물
		}
		CropLogic::UpdateGrowth(crop, *data, dtMs);
	}
}
