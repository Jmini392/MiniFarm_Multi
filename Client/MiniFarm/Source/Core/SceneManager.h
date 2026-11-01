#pragma once
#include "Scene.h"

class Shader;
class Camera;

class SceneManager
{
public:
    static void Init();

    static void Update(int);
    static void Draw();
    static void Reshape(int w, int h);
    
    static void SetScene(std::unique_ptr<Scene> newScene);
    static Scene* GetScene() { return s_currentScene.get(); }

    static Camera& GetCamera();
    static Shader& GetMainShader();
    static std::vector<std::shared_ptr<GameObject>>& GetObjects();
    static void InputTestKey(char key);
private:
    static std::unique_ptr<Scene> s_currentScene;
    static Shader s_mainShader;
    static int s_prevTime;
};
