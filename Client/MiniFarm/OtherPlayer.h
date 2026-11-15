#pragma once

#include "Player.h"

class OtherPlayer : public Player
{
public:
    OtherPlayer(int playerId, SharedWallet& wallet);

    void SetMoveTarget(const glm::vec3& position, float yaw, bool isMoving, bool isAirborne);

    void Update(int time) override;

private:
    glm::vec3 m_startPosition{ 0.f };
    glm::vec3 m_targetPosition{ 0.f };

    float m_startYaw = 0.f;
    float m_targetYaw = 0.f;

    float m_moveTime = 0.f;
    static constexpr float MOVE_DURATION = 0.05f;

    bool m_isMoving = false;
    bool m_isAirborne = false;
    bool m_hasMoveState = false;
};