#include "GameWorld.h"
#include <cstdio>
#include <string>

bool GameWorld::Init(const char* cropCsvPath)
{
	// 작물 데이터 읽기
	std::string error;
	if (!m_cropTable.Load(cropCsvPath, error)) {
		printf("[월드] 작물 데이터 로딩 실패: %s\n", error.c_str());
		return false;
	}
	return true;
}

void GameWorld::AddPlayer(int sessionId)
{
	m_inventories[sessionId]; // 없으면 빈 인벤토리를 만든다
}

void GameWorld::RemovePlayer(int sessionId)
{
	m_inventories.erase(sessionId);
}

InventoryCore* GameWorld::GetInventory(int sessionId)
{
	auto it = m_inventories.find(sessionId);
	if (it == m_inventories.end()) {
		return nullptr;
	}
	return &it->second;
}

bool GameWorld::AddField(int objectId)
{
	if (objectId < 0) {
		return false;
	}

	FieldState field;
	field.objectId = objectId;
	return m_fields.emplace(objectId, field).second;
}

FieldState* GameWorld::GetField(int objectId)
{
	auto it = m_fields.find(objectId);
	if (it == m_fields.end()) {
		return nullptr;
	}
	return &it->second;
}
