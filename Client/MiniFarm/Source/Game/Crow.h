#pragma once
#include "GameObject.h"

// 서버가 움직이는 까마귀. 서버에서 받은 ID와 위치로 만든다.
// 몸통과 양쪽 날개를 따로 그려서, 날 때는 날개를 퍼덕이고 밭에 앉으면 접는다.
// 모델은 원본 crow.obj를 crow_body / crow_wing_left / crow_wing_right로 나눈 것이다.
// 텍스처가 아직 없어 검은 단색 이미지를 쓴다 (까마귀 텍스처가 나오면 교체)
class Crow : public GameObject {
public:
	static constexpr const char* BODY_MODEL_PATH = "Models/crow_body.obj";
	static constexpr const char* LEFT_WING_MODEL_PATH = "Models/crow_wing_left.obj";   // +x 쪽 날개
	static constexpr const char* RIGHT_WING_MODEL_PATH = "Models/crow_wing_right.obj"; // -x 쪽 날개
	static constexpr const char* TEXTURE_PATH = "Assets/black.png";

	// 서버가 까마귀 위치를 보내는 간격(초). 이 시간 동안 이전 위치에서 새 위치로 옮겨 간다
	static constexpr float MOVE_DURATION = 0.1f;

	Crow(int netId, const glm::vec3& pos, float yaw = 0.f, const glm::vec3& scale = glm::vec3(1.f));

	// 서버에서 받은 새 위치와 방향. 바로 옮기지 않고 Update에서 부드럽게 따라간다.
	// isFlying은 서버의 isMoving 값이다 (밭에 앉아 있으면 false)
	void SetMoveTarget(const glm::vec3& pos, float yaw, bool isFlying);

	void Update(int time) override;
	void Draw() override;

private:
	void UpdateWingAnimation(float dt);

	std::shared_ptr<Model> m_leftWing;
	std::shared_ptr<Model> m_rightWing;

	glm::vec3 m_startPos;
	glm::vec3 m_targetPos;
	float m_startYaw = 0.f;
	float m_targetYaw = 0.f;
	float m_moveTime = MOVE_DURATION;

	bool m_isFlying = true;  // 접속 직후에는 날고 있다고 보고, 다음 위치 패킷에서 바로잡는다
	float m_flapTime = 0.f;  // 퍼덕임 주기 계산용 누적 시간(초)
	float m_wingSpread = 1.f; // 0이면 날개를 접고, 1이면 완전히 펴서 퍼덕인다
	float m_wingAngle = 0.f; // 몸통에 붙은 상태에서 바깥으로 벌린 각도(도)
};
