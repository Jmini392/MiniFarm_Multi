#include "PCH.h"
#include "TitleScene.h"
#include "TextRenderer.h"
#include "SceneManager.h"
#include "IngameScene.h"
#include "InputManager.h"
#include "ObjectLoader.h"
#include "UIRenderer.h"
#include "NetworkClient.h"
#include "Shared/Protocol.h"

void TitleScene::Init()
{
	m_objects.clear();

	m_camera.yaw = 30.0f;
	m_camera.pitch = 20.0f;
	m_camera.distance = 60.0f;
	m_camera.FollowTarget(glm::vec3(0.f));

	auto staticObjs = ObjectLoader::LoadStaticObjects("Data/static_props_pos.json");
	auto interactObjs = ObjectLoader::LoadInteractableObjects("../../Shared/Data/Interactable_props_pos.json");

	for (auto& obj : staticObjs)
		AddObject(obj);

	for (auto& obj : interactObjs)
		AddObject(obj);
}

void TitleScene::Update(int dt)
{
	bool StartPressed = InputManager::IsKeyPressed(' ');

	// 타이틀에서도 이전 키 상태를 갱신한다.
	// 접속 실패 후 키를 놓고 다시 누르면 재시도할 수 있다.
	InputManager::Update();

	auto& networkClient = SceneManager::GetNetworkClient();

	if (m_connectRequested)
	{
		m_connectRequested = false;

		if (!networkClient.Connect(m_serverIp.c_str(), SERVER_PORT))
		{
			m_connectionMessage = "Connection failed";
		}
		else if (!networkClient.SendLoginRequest())
		{
			networkClient.Disconnect();
			m_connectionMessage = "Login request failed";
		}
		else
		{
			m_connectionMessage = "Waiting for login";
			m_loginWaitMs = 5000;
		}
	}
	else if (m_loginWaitMs > 0)
	{
		m_loginWaitMs -= dt;
		if (m_loginWaitMs <= 0)
		{
			networkClient.Disconnect();
			m_connectionMessage = "Login response timed out";
		}
	}

	m_blinkTime += dt * 0.001f;
	m_camera.AddYaw(0.01f * dt);
	m_camera.FollowTarget(glm::vec3(0.f));
}

void TitleScene::Draw()
{
	SetClearColor(glm::vec3(0.5, 0.8, 1.0));

	SetupCameraAndLight();
	DrawWorld();

	UIRenderer::DrawCenter(TextureLoader::Load("Assets/ui_title.png"), { 0.5f,0.5f }, 1.0f, { 1,1,1,1 });

	TextRenderer::Draw("Server IP: " + m_serverIp,
		WINDOW_W / 2, WINDOW_H / 2 - 50, 2, { 1, 1, 1 });

	TextRenderer::Draw("Press Space to connect",
		WINDOW_W / 2, WINDOW_H / 2 - 85, 1.5f, { 1, 1, 1 });

	if (!m_connectionMessage.empty())
	{
		TextRenderer::Draw(m_connectionMessage,
			WINDOW_W / 2, WINDOW_H / 2 - 115, 1.5f, { 1, 1, 1 });
	}
}

void TitleScene::DrawWorld()
{
	for (auto& obj : m_objects)
	{
		obj->DebugDraw();
		obj->Draw();
	}
}

void TitleScene::SetupCameraAndLight()
{
	Shader* shader = &SceneManager::GetMainShader();

	shader->Use();
	shader->SetView(m_camera.GetView());
	shader->SetProj(m_camera.GetProj((float)WINDOW_W / WINDOW_H));

	shader->SetLightPos(glm::vec3(10.f, 15.f, 10.f));
	shader->SetLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
	shader->SetViewPos(m_camera.eye);

	shader->SetModel(glm::mat4(1.0f));
}

void TitleScene::OnKeyDown(unsigned char key)
{
	if (m_connectRequested || m_loginWaitMs > 0)
		return;

	if (key == ' ')
	{
		m_connectRequested = true;
	}
	else if (key == '\b')
	{
		if (!m_serverIp.empty())
			m_serverIp.pop_back();
		m_connectionMessage.clear();
	}
	else if (((key >= '0' && key <= '9') || key == '.')
		&& m_serverIp.size() < 15)
	{
		m_serverIp.push_back(static_cast<char>(key));
		m_connectionMessage.clear();
	}
}