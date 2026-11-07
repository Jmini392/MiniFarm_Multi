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
	m_players.try_emplace(sessionId, sessionId);
}

void GameWorld::RemovePlayer(int sessionId)
{
	m_players.erase(sessionId);
}

PlayerState* GameWorld::GetPlayer(int sessionId)
{
	auto it = m_players.find(sessionId);

	if (it == m_players.end())
		return nullptr;

	return &it->second;
}

InventoryCore* GameWorld::GetInventory(int sessionId)
{
	PlayerState* player = GetPlayer(sessionId);

	if (player == nullptr)
		return nullptr;

	return &player->GetInventory();
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
