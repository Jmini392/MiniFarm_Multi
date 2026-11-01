#include "PCH.h"
#include "ExitScene.h"
#include "TextRenderer.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "UIRenderer.h"
#include "ObjectLoader.h"
#include "Boat.h"

void ExitScene::Init()
{
	m_objects.clear();

	m_camera.yaw = 0.0f;
	m_camera.pitch = 5.0f;
	m_camera.distance = 60.0f;
	m_camera.FollowTarget(glm::vec3(0.f));

	auto objs = ObjectLoader::LoadEndingObjects();

	for (auto& obj : objs)
	{
		if (auto boat = std::dynamic_pointer_cast<Boat>(obj))
			m_boat = boat;
		AddObject(obj);
	}

}

void ExitScene::Update(int dt)
{
	m_boat->m_pos.z -= 1.0f;
	m_boat->m_pos.y -= 0.01f;
	m_boat->m_rot.z = sin(dt / 1000.f * 1.5f) * 30.f;

	m_blinkTime += dt * 0.001f;
	m_startFade = true;

	if (m_startFade)
	{
		m_fade += dt * 0.0001f;
		if (m_fade >= 1.0f)
		{
			exit(0);
		}
	}
}

void ExitScene::Draw()
{
	SetClearColor(glm::vec3(0.5, 0.8, 1.0));

	SetupCameraAndLight();
	DrawWorld();

	UIRenderer::DrawCenter(TextureLoader::Load("Assets/ui_end.png"), { 0.5f,0.5f }, 1.0f, { 1,1,1,1 });

	if (sin(m_blinkTime * 4.0f) > 0.0f)
		TextRenderer::Draw("Congratulations on your escape!", WINDOW_W / 2, WINDOW_H / 2 - 50, 2, glm::vec3(0.f));
	if (m_startFade)
	{
		glm::vec4 fadeColor = { 0, 0, 0, glm::clamp(m_fade, 0.f, 1.f) };
		UIRenderer::DrawCenter(TextureLoader::Load("Assets/black.png"), { 0.5f,0.5f }, 2.0f, fadeColor);
	}
}

void ExitScene::DrawWorld()
{
	for (auto& obj : m_objects)
	{
		obj->DebugDraw();
		obj->Draw();
	}
}

void ExitScene::SetupCameraAndLight()
{
	Shader* shader = &SceneManager::GetMainShader();

	shader->Use();
	shader->SetView(m_camera.GetView());
	shader->SetProj(m_camera.GetProj((float)WINDOW_W / WINDOW_H));

	shader->SetLightPos(glm::vec3(50.f));
	shader->SetLightColor(glm::vec3(0.8f));
	shader->SetViewPos(m_camera.eye);

	shader->SetModel(glm::mat4(1.0f));
}
