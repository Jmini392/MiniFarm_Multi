#include "PCH.h"
#include "TitleScene.h"
#include "TextRenderer.h"
#include "SceneManager.h"
#include "IngameScene.h"
#include "InputManager.h"
#include "ObjectLoader.h"
#include "UIRenderer.h"
#include "NetworkClient.h"

void TitleScene::Init()
{
	m_objects.clear();

	m_camera.yaw = 30.0f;
	m_camera.pitch = 20.0f;
	m_camera.distance = 60.0f;
	m_camera.FollowTarget(glm::vec3(0.f));

	auto staticObjs = ObjectLoader::LoadStaticObjects("Data/static_props_pos.json");
	auto interactObjs = ObjectLoader::LoadInteractableObjects("Data/Interactable_props_pos.json");

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

	if (StartPressed)
	{
		auto& networkClient = SceneManager::GetNetworkClient();

		if (networkClient.Connect("127.0.0.1", 9000))
		{
			// 연결 성공 후 로그인 요청
			if (!networkClient.SendLoginRequest())
			{
				LOG_E("로그인 요청 전송 실패");
				networkClient.Disconnect();
			}
			else
			{
				LOG("서버 로그인 응답 대기");
			}
		}
		else
		{
			LOG_E("서버 접속 실패");
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

	if (sin(m_blinkTime * 4.0f) > 0.0f)
	{
		TextRenderer::Draw("Press [SpaceBar] to start", WINDOW_W / 2, WINDOW_H / 2 - 50, 2, { 1,1,1 });
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
