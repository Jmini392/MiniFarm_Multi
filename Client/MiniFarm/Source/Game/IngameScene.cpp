#include "PCH.h"
#include "IngameScene.h"
#include "SceneManager.h"
#include "UIRenderer.h"
#include "TextRenderer.h"
#include "ObjectLoader.h"
#include "Shader.h"
#include "ExitScene.h"
#include "WaterParticleSystem.h"

void IngameScene::Init()
{
	SetClearColor(glm::vec3(1, 1, 1));

	m_objects.clear();
	m_camera.Init();
	auto player = std::make_shared<Player>();
	m_player = player.get();
	AddObject(player);

	auto staticObjs = ObjectLoader::LoadStaticObjects("Data/static_props_pos.json");
	auto interactObjs = ObjectLoader::LoadInteractableObjects("Data/Interactable_props_pos.json");

	for (auto& obj : staticObjs)
		AddObject(obj);

	for (auto& obj : interactObjs)
	{
		if (auto crop = std::dynamic_pointer_cast<Crop>(obj))
			m_crops.push_back(crop);

		AddObject(obj);
	}
	WaterParticleSystem::Init();
}

void IngameScene::Update(int dt)
{
	for (auto& obj : m_objects)
	{
		if (!m_valid) return;
		obj->Update(dt);
	}
	if(m_player->IsOnBoat())
	{
		SceneManager::SetScene(std::make_unique<ExitScene>());
		return;
	}
	m_timeSystem.Update(dt);
	UpdateDayNightCycle();
	SetClearColor(m_BackGroundColor);
	WaterParticleSystem::Update(static_cast<float>(dt));
}

void IngameScene::Draw()
{
	SetupCameraAndLight();
	DrawWorld();
	Shader& mainShader = SceneManager::GetMainShader();
	WaterParticleSystem::Draw(&mainShader);
	DrawUI();
}

void IngameScene::DrawWorld()
{
	for (auto& obj : m_objects)
	{
		obj->DebugDraw();
		obj->Draw();
	}
}

void IngameScene::DrawUI()
{
	for (auto& crop : m_crops)
		crop->DrawBar();

	if (m_player->IsShopping() || m_player->IsEscaping())
	{
		UIRenderer::DrawMessage(m_player->GetSysMsg());
	}
	else if (m_player->HasInteractTarget())
	{
		UIRenderer::DrawCenter(TextureLoader::Load("Assets/ui_board_black_thin.png"), { 0.5f,0.6f }, 0.6f, { 1,1,1,0.6 });
		TextRenderer::Draw("Press [E] to interact", WINDOW_W / 2, WINDOW_H / 2 + 50, 2, { 0,1,0.2 });
	}

	m_player->GetInventory().DrawUI();

	float timeValue = m_timeSystem.GetDayTime();
	int hour = (int)timeValue;
	int minute = (int)((timeValue - hour) * 60.f);
	std::string ampm = (hour < 12) ? "AM" : "PM";
	UIRenderer::DrawCenter(TextureLoader::Load("Assets/ui_timebar.png"), { 0.8f,0.92f }, 0.25f, { 1,1,1,1 });
	TextRenderer::Draw(std::format("{:02d}:{:02d} {}", hour, minute, ampm), WINDOW_W - 85, WINDOW_H - 40, 1.3f, { 0,0,0 });
}

void IngameScene::SetupCameraAndLight()
{
	Shader* shader = &SceneManager::GetMainShader();
	shader->Use();
	shader->SetView(m_camera.GetView());
	shader->SetProj(m_camera.GetProj((float)WINDOW_W / WINDOW_H));

	shader->SetLightPos(m_sunPos);
	shader->SetLightColor(m_sunColor);
	shader->SetVec3(m_moonPos, "uMoonPos");
	shader->SetVec3(m_moonColor, "uMoonColor");
	shader->SetViewPos(m_camera.eye);

	shader->SetModel(glm::mat4(1.0f));
}

void IngameScene::UpdateDayNightCycle()
{
	float dayTime = m_timeSystem.GetDayTime();
	const float PI = 3.14159265f;

	float rotationAngle = ((dayTime - 6.f) / 24.f) * 2.f * PI;

	m_sunPos.x = cos(rotationAngle) * LIGHT_WIDTH;
	m_sunPos.y = sin(rotationAngle) * LIGHT_HEIGHT;
	m_sunPos.z = 0.f;

	m_moonPos.x = cos(rotationAngle + PI) * LIGHT_WIDTH;
	m_moonPos.y = sin(rotationAngle + PI) * LIGHT_HEIGHT;
	m_moonPos.z = 0.f;

	float sunHeight = m_sunPos.y / LIGHT_HEIGHT;

	if (sunHeight > 0.1f)
	{
		float t = glm::clamp((sunHeight - 0.1f) / 0.2f, 0.f, 1.f);
		m_sunColor = glm::mix(glm::vec3(1.f, 0.6f, 0.3f), glm::vec3(1.f, 0.95f, 0.8f), t);
		m_moonColor = glm::vec3(0.1f, 0.1f, 0.2f);
		m_BackGroundColor = glm::mix(glm::vec3(0.4f, 0.4f, 0.6f), glm::vec3(0.5f, 0.8f, 1.f), t);
	}
	else if (sunHeight > -0.1f)
	{
		float t = (sunHeight + 0.1f) / 0.2f;
		m_sunColor = glm::mix(glm::vec3(0.3f, 0.2f, 0.3f), glm::vec3(1.f, 0.6f, 0.3f), t);
		m_moonColor = glm::mix(glm::vec3(0.4f, 0.5f, 0.7f), glm::vec3(0.1f, 0.1f, 0.2f), t);
		m_BackGroundColor = glm::mix(glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(0.4f, 0.4f, 0.6f), t);
	}
	else
	{
		float t = glm::clamp((-sunHeight - 0.1f) / 0.2f, 0.f, 1.f);
		m_sunColor = glm::vec3(0.1f, 0.1f, 0.1f);
		m_moonColor = glm::mix(glm::vec3(0.3f, 0.4f, 0.5f), glm::vec3(0.4f, 0.5f, 0.7f), t);
		m_BackGroundColor = glm::mix(glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(0.05f, 0.05f, 0.15f), t);
	}
}

void IngameScene::TestAddMoney(int amount)
{
	m_player->AddMoney(amount);
}
