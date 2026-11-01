#pragma once
#include "Scene.h"

class TitleScene : public Scene
{
public:
    void Init() override;
    void Update(int dt) override;
    void Draw() override;

    void DrawWorld();
    void SetupCameraAndLight();
private:
    float m_blinkTime = 0.f;
};
