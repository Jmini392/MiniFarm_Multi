#include "PCH.h"
#include "OtherPlayer.h"

OtherPlayer::OtherPlayer(int playerId, SharedWallet& wallet) : Player(wallet)
{
    m_netId = playerId;
    m_name = "OtherPlayer";

    m_collider.reset();
}

void OtherPlayer::SetMoveTarget(const glm::vec3& position, float yaw, bool isMoving, bool isAirborne)
{
    m_targetYaw = std::remainder(yaw, 360.f);
    m_targetPosition = position;
    m_isMoving = isMoving;
    m_isAirborne = isAirborne;

    // 현재 위치와 받은 위치가 너무 멀면 보간 없이 옮김
    constexpr float TELEPORT_DISTANCE = 10.f;
    if (!m_hasMoveState || glm::distance(m_pos, position) > TELEPORT_DISTANCE)
    {
        m_pos = m_targetPosition;
        m_rot.y = m_targetYaw;
    }

    // 현재 표시 상태에서 새로운 보간을 시작한다.
    m_startPosition = m_pos;
    m_startYaw = std::remainder(m_rot.y, 360.f);
    m_moveTime = 0.f;

    m_hasMoveState = true;
}

void OtherPlayer::Update(int time)
{
    float dt = time / 1000.f;

    // 캐릭터 이동 선형 보간
    if (m_hasMoveState)
    {
        // 경과 시간을 누적하되 보간 시간을 넘지 않게 한다.
        m_moveTime = std::clamp(m_moveTime + dt, 0.f, MOVE_DURATION);

        float ratio = m_moveTime / MOVE_DURATION;

        // 고정된 시작 위치와 목표 위치 사이를 계산한다.
        m_pos = glm::mix(m_startPosition, m_targetPosition, ratio);

        // 시작 방향에서 목표 방향까지 짧은 쪽으로 회전한다.
        float yawDifference = std::remainder(m_targetYaw - m_startYaw, 360.f);
        m_rot.y = std::remainder(m_startYaw + yawDifference * ratio, 360.f);

        m_isWalking = m_isMoving;
        UpdateWalkAnimations();
    }

    UpdateToolAnimation(time);
    GameObject::Update(time);
}