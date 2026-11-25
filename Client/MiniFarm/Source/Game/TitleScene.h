#pragma once
#include "Scene.h"
#include <string>

class TitleScene : public Scene
{
public:
    void Init() override;
    void Update(int dt) override;
    void Draw() override;

    void DrawWorld();
    void SetupCameraAndLight();

    void OnKeyDown(unsigned char key);
private:
    //std::string m_serverIp;
    std::string m_serverIp = "127.0.0.1";
    std::string m_connectionMessage;
    bool m_connectRequested = false;
    int m_loginWaitMs = 0;

    float m_blinkTime = 0.f;
};
