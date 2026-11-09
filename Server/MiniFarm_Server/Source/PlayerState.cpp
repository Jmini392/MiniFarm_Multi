#include "PlayerState.h"

#include <cmath>

PlayerState::PlayerState(int playerId) : m_id(playerId)
{
    m_inventory.AddItem(EItemID::SeedCarrot, 1);
    m_inventory.AddItem(EItemID::SeedCabbage, 1);
    m_inventory.AddItem(EItemID::Carrot, 1);
    m_inventory.AddItem(EItemID::Cabbage, 1);
}

bool PlayerState::SetMovement(float x, float y, float z, float yaw, bool isMoving, bool isAirborne)
{
    // NaN이나 무한대가 위치에 저장되는 것을 방지
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(yaw))
    {
        return false;
    }

    m_position.x = x;
    m_position.y = y;
    m_position.z = z;

    m_yaw = yaw;
    m_isMoving = isMoving;
    m_isAirborne = isAirborne;

    return true;
}

bool PlayerState::SetEquippedTool(EItemID item)
{
    switch (item)
    {
    case EItemID::Empty:
    case EItemID::Hoe:
    case EItemID::WateringCan:
    case EItemID::Sickle:
    case EItemID::SeedCarrot:
    case EItemID::SeedCabbage:
        m_equippedTool = item;
        return true;

    default:
        return false;
    }
}