#include "PCH.h"
#include "Crow.h"
#include "SceneManager.h"
#include "Shader.h"

namespace {
	// 날개가 몸통에 붙는 곳 (모델 좌표). 날개 윗변이 몸통 옆면과 만나는 선 위의 점이다
	const glm::vec3 LEFT_WING_JOINT = { 0.31f, 1.23f, 0.f };
	const glm::vec3 RIGHT_WING_JOINT = { -0.31f, 1.23f, 0.f };

	constexpr float FLAP_PER_SECOND = 3.f;  // 초당 퍼덕이는 횟수
	constexpr float FLAP_CENTER = 90.f;     // 날개를 수평으로 편 각도 (0이면 접힘)
	constexpr float FLAP_AMPLITUDE = 40.f;  // 수평 기준 위아래로 움직이는 폭
	constexpr float WING_SPREAD_SPEED = 4.f; // 날개를 펴거나 접는 속도 (초당 비율)
	constexpr float TWO_PI = 6.2831853f;
}

Crow::Crow(int netId, const glm::vec3& pos, float yaw, const glm::vec3& scale)
	: GameObject(pos, glm::vec3(0.f), scale)
{
	m_netId = netId;
	m_name = "Crow";
	m_model = ModelCache::Get(BODY_MODEL_PATH, TEXTURE_PATH);
	m_leftWing = ModelCache::Get(LEFT_WING_MODEL_PATH, TEXTURE_PATH);
	m_rightWing = ModelCache::Get(RIGHT_WING_MODEL_PATH, TEXTURE_PATH);
	m_rot.y = yaw;
	m_startPos = m_targetPos = pos;
	m_startYaw = m_targetYaw = yaw;
}

void Crow::SetMoveTarget(const glm::vec3& pos, float yaw, bool isFlying)
{
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
	m_isFlying = isFlying;
}

void Crow::Update(int time)
{
	const float dt = time / 1000.f;

	m_moveTime = std::min(m_moveTime + dt, MOVE_DURATION);
	const float ratio = m_moveTime / MOVE_DURATION;
	m_pos = glm::mix(m_startPos, m_targetPos, ratio);
	// 짧은 쪽으로 돈다
	const float yawDiff = std::remainder(m_targetYaw - m_startYaw, 360.f);
	m_rot.y = std::remainder(m_startYaw + yawDiff * ratio, 360.f);

	UpdateWingAnimation(dt);
	GameObject::Update(time);
}

void Crow::UpdateWingAnimation(float dt)
{
	// 날고 있으면 날개를 펴고, 밭에 앉으면 천천히 접는다
	const float targetSpread = m_isFlying ? 1.f : 0.f;
	const float step = WING_SPREAD_SPEED * dt;
	if (m_wingSpread < targetSpread) m_wingSpread = std::min(m_wingSpread + step, targetSpread);
	else m_wingSpread = std::max(m_wingSpread - step, targetSpread);

	m_flapTime = std::fmod(m_flapTime + dt, 1.f / FLAP_PER_SECOND);
	const float flap = std::sin(m_flapTime * FLAP_PER_SECOND * TWO_PI);
	m_wingAngle = m_wingSpread * (FLAP_CENTER + FLAP_AMPLITUDE * flap);
}

void Crow::Draw()
{
	GameObject::Draw(); // 몸통

	const glm::mat4 crowTransform =
		Translate(m_pos)
		* Rotate(m_rot.y, { 0,1,0 })
		* Rotate(m_rot.x, { 1,0,0 })
		* Rotate(m_rot.z, { 0,0,1 })
		* Scale(m_scale);

	auto& shader = SceneManager::GetMainShader();

	// 날개는 몸통에 붙은 윗변을 축으로 앞뒤(z) 방향 축을 따라 바깥쪽으로 돈다
	if (m_leftWing) {
		shader.SetModel(crowTransform
			* Translate(LEFT_WING_JOINT) * Rotate(m_wingAngle, { 0,0,1 }) * Translate(-LEFT_WING_JOINT));
		m_leftWing->Draw();
	}
	if (m_rightWing) {
		shader.SetModel(crowTransform
			* Translate(RIGHT_WING_JOINT) * Rotate(-m_wingAngle, { 0,0,1 }) * Translate(-RIGHT_WING_JOINT));
		m_rightWing->Draw();
	}
}
