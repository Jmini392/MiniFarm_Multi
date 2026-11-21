#pragma once
#include "GameObject.h"

// 서버가 움직이는 까마귀. 서버에서 받은 ID와 위치로 만든다.
// 텍스처가 아직 없어 검은 단색 이미지를 쓴다 (까마귀 텍스처가 나오면 교체)
class Crow : public GameObject {
public:
	static constexpr const char* MODEL_PATH = "Models/crow.obj";
	static constexpr const char* TEXTURE_PATH = "Assets/black.png";

	// 서버가 까마귀 위치를 보내는 간격(초). 이 시간 동안 이전 위치에서 새 위치로 옮겨 간다
	static constexpr float MOVE_DURATION = 0.1f;

	Crow(int netId, const glm::vec3& pos, float yaw = 0.f) : GameObject(pos) {
		m_netId = netId;
		m_name = "Crow";
		m_model = ModelCache::Get(MODEL_PATH, TEXTURE_PATH);
		m_rot.y = yaw;
		m_startPos = m_targetPos = pos;
		m_startYaw = m_targetYaw = yaw;
	}

	// 서버에서 받은 새 위치와 방향. 바로 옮기지 않고 Update에서 부드럽게 따라간다
	void SetMoveTarget(const glm::vec3& pos, float yaw) {
		// 너무 멀리 떨어졌으면 보간하지 않고 바로 옮긴다
		constexpr float TELEPORT_DISTANCE = 10.f;
		if (glm::distance(m_pos, pos) > TELEPORT_DISTANCE) {
			m_pos = pos;
			m_rot.y = yaw;
		}
		m_startPos = m_pos;
		m_startYaw = std::remainder(m_rot.y, 360.f);
		m_targetPos = pos;
		m_targetYaw = std::remainder(yaw, 360.f);
		m_moveTime = 0.f;
	}

	void Update(int time) override {
		m_moveTime = std::min(m_moveTime + time / 1000.f, MOVE_DURATION);
		const float ratio = m_moveTime / MOVE_DURATION;
		m_pos = glm::mix(m_startPos, m_targetPos, ratio);
		// 짧은 쪽으로 돈다
		const float yawDiff = std::remainder(m_targetYaw - m_startYaw, 360.f);
		m_rot.y = std::remainder(m_startYaw + yawDiff * ratio, 360.f);
		GameObject::Update(time);
	}

private:
	glm::vec3 m_startPos;
	glm::vec3 m_targetPos;
	float m_startYaw = 0.f;
	float m_targetYaw = 0.f;
	float m_moveTime = MOVE_DURATION;
};
