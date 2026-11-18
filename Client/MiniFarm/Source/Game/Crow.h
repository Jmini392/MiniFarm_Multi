#pragma once
#include "GameObject.h"

// 서버가 움직이는 까마귀. 서버에서 받은 ID와 위치로 만든다.
// 텍스처가 아직 없어 검은 단색 이미지를 쓴다 (까마귀 텍스처가 나오면 교체)
class Crow : public GameObject
{
public:
	static constexpr const char* MODEL_PATH = "Models/crow.obj";
	static constexpr const char* TEXTURE_PATH = "Assets/black.png";

	Crow(int netId, const glm::vec3& pos)
		: GameObject(pos)
	{
		m_netId = netId;
		m_name = "Crow";
		m_model = ModelCache::Get(MODEL_PATH, TEXTURE_PATH);
	}
};
