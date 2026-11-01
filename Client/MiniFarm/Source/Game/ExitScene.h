#pragma once
#include "Scene.h"

class ExitScene : public Scene
{
public:
	void Init() override;
	void Update(int dt) override;
	void Draw() override;

	void DrawWorld();
	void SetupCameraAndLight();
private:
	std::shared_ptr<GameObject> m_boat;
	float m_blinkTime = 0.f;
	float m_fade = 0.0f;
	bool m_startFade = false;
};
