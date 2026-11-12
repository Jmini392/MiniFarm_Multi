#pragma once
#include "Scene.h"
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"
#include "Shader.h"
#include "TimeSystem.h"
#include "Crop.h"
#include "Common/SharedWallet.h"
#include <unordered_map>

class OtherPlayer;

struct S2C_LOGIN_INFO_PACKET;
struct S2C_ADD_OBJECT_PACKET;
struct S2C_MOVE_OBJECT_PACKET;
struct S2C_FIELD_UPDATE_PACKET;

class IngameScene : public Scene
{
public:
	void Init() override;
	void Update(int dt) override;
	void Draw() override;

	void DrawWorld();
	void DrawUI();
	void SetupCameraAndLight();
	void UpdateDayNightCycle();

	void TestAddMoney(int amount);

	void ApplyLoginInfo(const S2C_LOGIN_INFO_PACKET& info);
	bool AddWorldObject(const S2C_ADD_OBJECT_PACKET& packet);
	bool ApplyMoveObject(const S2C_MOVE_OBJECT_PACKET& packet);
	void RemoveWorldObject(int objectId);
	bool ApplyFieldUpdate(const S2C_FIELD_UPDATE_PACKET& packet);
private:
	int m_playerId = -1;

	SharedWallet m_wallet; // Local world wallet until server synchronization is connected.
	Player* m_player = nullptr;
	TimeSystem m_timeSystem;
	std::vector<std::shared_ptr<Crop>> m_crops;

	struct MoveState
	{
		glm::vec3 position{ 0.f };
		float yaw = 0.f;
		bool isMoving = false;
		bool isAirborne = false;
	};

	std::unordered_map<int, MoveState> m_moveStates;
	std::unordered_map<int, std::shared_ptr<OtherPlayer>> m_OtherPlayers;

	const float LIGHT_WIDTH = 20.f;
	const float LIGHT_HEIGHT = 40.f;
	glm::vec3 m_BackGroundColor = glm::vec3(1.f,1.f,1.f);
	glm::vec3 m_sunPos = glm::vec3(-15.f, 0.f, 0.f);
	glm::vec3 m_sunColor = glm::vec3(1.f, 0.95f, 0.8f);
	glm::vec3 m_moonPos = glm::vec3(15.f, 0.f, 0.f);
	glm::vec3 m_moonColor = glm::vec3(0.4f, 0.5f, 0.7f);
};
