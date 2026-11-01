#include "PCH.h"
#include "SceneManager.h"
#include "IngameScene.h"
#include "TitleScene.h"
#include "InputManager.h"

std::unique_ptr<Scene> SceneManager::s_currentScene = nullptr;
int SceneManager::s_prevTime = 0;
Shader SceneManager::s_mainShader;
void SceneManager::Init()
{
	SetScene(std::make_unique<TitleScene>());
	s_mainShader.Init("Shaders/main.vert", "Shaders/main.frag");

}

void SceneManager::Update(int)
{
	int now = glutGet(GLUT_ELAPSED_TIME);
	int dt = now - s_prevTime;
	s_prevTime = now;

	if (dt < 0) dt = 0;

	if (s_currentScene && s_currentScene->IsValid())
		s_currentScene->Update(dt);

	glutPostRedisplay();
	glutTimerFunc(FRAME_TIME_MS, SceneManager::Update, 0);
}

void SceneManager::Draw()
{
	glm::vec3 color = s_currentScene->GetClearColor();
	glClearColor(color.r, color.g, color.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (s_currentScene && s_currentScene->IsValid())
		s_currentScene->Draw();

	glutSwapBuffers();
}

void SceneManager::Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void SceneManager::SetScene(std::unique_ptr<Scene> newScene)
{
	InputManager::Clear();
	if (s_currentScene && s_currentScene->IsValid())
		s_currentScene->SetValid(false);
	 
	s_currentScene = std::move(newScene);

	if (s_currentScene)
	{
		s_currentScene->Init();
		s_currentScene->SetValid(true);
	}
	s_prevTime = glutGet(GLUT_ELAPSED_TIME);

	glUseProgram(0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

Camera& SceneManager::GetCamera()
{
	return s_currentScene->GetCamera();
}

void SceneManager::InputTestKey(char key)
{
	switch (key)
	{
	case 't':
	{
		static bool bShowCursor = false;
		bShowCursor = !bShowCursor;
		InputManager::SetUIMode(bShowCursor);
		break;
	}
	case 'c':
	{
		if (auto* s = dynamic_cast<IngameScene*>(s_currentScene.get()))
		{
			s->TestAddMoney(GOAL_MONEY);
		}

		break;
	}
	case 27: exit(0); //ESC
	}
}

Shader& SceneManager::GetMainShader()
{
	return s_mainShader;
}

std::vector<std::shared_ptr<GameObject>>& SceneManager::GetObjects()
{
	return s_currentScene->GetObjects();
}