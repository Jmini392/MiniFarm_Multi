#pragma once

#include "../../../Common/InventoryCore.h"

// 서버가 보관할 플레이어 위치
struct PlayerPosition
{
    float x = 0.f;
    float y = 8.f;
    float z = 0.f;
};

class PlayerState
{
public:
    explicit PlayerState(int playerId);

    int GetId() const { return m_id; }

    const PlayerPosition& GetPosition() const
    {
        return m_position;
    }

    float GetYaw() const { return m_yaw; }

    bool IsMoving() const { return m_isMoving; }
    bool IsAirborne() const { return m_isAirborne; }

    EItemID GetEquippedTool() const
    {
        return m_equippedTool;
    }

    // 위치, 몸 방향, 애니메이션 상태를 함께 변경
    bool SetMovement(
        float x, float y, float z,
        float yaw,
        bool isMoving,
        bool isAirborne
    );

    bool SetEquippedTool(EItemID item);

    bool IsFieldSyncReady() const { return m_fieldSyncReady; }
    void SetFieldSyncReady(bool ready) { m_fieldSyncReady = ready; }

    InventoryCore& GetInventory()
    {
        return m_inventory;
    }

private:
    bool m_fieldSyncReady = false; // 초기 밭 상태 전송이 끝나야 변경 상태를 받는다.

    int m_id;                                   // 플레이어를 식별하는 ID

    PlayerPosition m_position;                  // 서버가 기억하는 현재 위치
    float m_yaw = 0.f;                          // Y축 몸 방향, 기존 클라이언트처럼 도 단위

    bool m_isMoving = false;                    // 걷기 애니메이션 등에 사용할 이동 상태
    bool m_isAirborne = false;                  // 공중에 있는 상태

    EItemID m_equippedTool = EItemID::Empty;    // 장착한 도구 또는 씨앗

    InventoryCore m_inventory;                  // 해당 플레이어의 개인 아이템
};