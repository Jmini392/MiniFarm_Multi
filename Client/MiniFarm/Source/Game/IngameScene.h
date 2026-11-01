#pragma once
#include "Scene.h"
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"
#include "Shader.h"
#include "TimeSystem.h"
#include "Crop.h"

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
private:
	Player* m_player;
	TimeSystem m_timeSystem;
	std::vector<std::shared_ptr<Crop>> m_crops;

	const float LIGHT_WIDTH = 20.f;
	const float LIGHT_HEIGHT = 40.f;
	glm::vec3 m_BackGroundColor = glm::vec3(1.f,1.f,1.f);
	glm::vec3 m_sunPos = glm::vec3(-15.f, 0.f, 0.f);
	glm::vec3 m_sunColor = glm::vec3(1.f, 0.95f, 0.8f);
	glm::vec3 m_moonPos = glm::vec3(15.f, 0.f, 0.f);
	glm::vec3 m_moonColor = glm::vec3(0.4f, 0.5f, 0.7f);
};
