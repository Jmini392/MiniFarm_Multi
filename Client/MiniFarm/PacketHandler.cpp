#include "PCH.h"
#include "PacketHandler.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "IngameScene.h"
#include "../../Shared/Protocol.h"

#include <cstring>

bool HandlePacket(const char* packet)
{
    if (packet == nullptr)
        return false;

    PacketHeader header;
    memcpy(&header, packet, sizeof(header));

    switch (header.type)
    {
    case S2C_LOGIN_INFO:      // 내 플레이어 초기화
    {
        if (header.size != sizeof(S2C_LOGIN_INFO_PACKET))
            return false;

        // 로그인 응답은 타이틀에서만 처리한다.
        if (dynamic_cast<TitleScene*>(SceneManager::GetScene()) == nullptr)
        {
            return false;
        }

        S2C_LOGIN_INFO_PACKET info;
        memcpy(&info, packet, sizeof(info));

        if (info.playerId <= 0 || !std::isfinite(info.x) || !std::isfinite(info.y) || !std::isfinite(info.z) || !std::isfinite(info.yaw))
        {
            return false;
        }

        if (info.carrotSeedCount < 0 || info.cabbageSeedCount < 0
            || info.carrotCount < 0 || info.cabbageCount < 0)
        {
            return false;
        }

        auto scene = std::make_unique<IngameScene>();
        IngameScene* ingame = scene.get();

        // 여기서 IngameScene::Init()이 실행된다.
        SceneManager::SetScene(std::move(scene));

        // 생성된 플레이어에 서버 정보를 적용한다.
        ingame->ApplyLoginInfo(info);

        LOG("로그인 완료: ID=%d, 위치=(%.1f, %.1f, %.1f)",
            (int)info.playerId, info.x, info.y, info.z);

        return true;
    }

    case S2C_ADD_OBJECT:
    {
        if (header.size != sizeof(S2C_ADD_OBJECT_PACKET))
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());
        if (ingame == nullptr)
            return false;

        S2C_ADD_OBJECT_PACKET add;
        memcpy(&add, packet, sizeof(add));

        return ingame->AddWorldObject(add);
    }

    case S2C_REMOVE_OBJECT:     // 월드 객체 삭제
        break;

    case S2C_MOVE_OBJECT:       // 객체 위치와 방향 반영
    {
        if (header.size != sizeof(S2C_MOVE_OBJECT_PACKET))
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());

        if (ingame == nullptr)
            return false;

        S2C_MOVE_OBJECT_PACKET move{};
        memcpy(&move, packet, sizeof(move));

        return ingame->ApplyMoveObject(move);
    }
    case S2C_EQUIP_OBJECT:        // 장착 도구 반영
        break;

    case S2C_FIELD_UPDATE:
    {
        if (header.size != sizeof(S2C_FIELD_UPDATE_PACKET))
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());
        if (ingame == nullptr)
            return false;

        S2C_FIELD_UPDATE_PACKET update{};
        memcpy(&update, packet, sizeof(update));

        return ingame->ApplyFieldUpdate(update);
    }

    case S2C_SHOP_BUY_RESULT:       // 구매 결과 반영
        break;

    case S2C_SHOP_SELL_RESULT:      // 판매 결과 반영
        break;

    case S2C_TIME_SYNC:             // 게임 시간 반영
        break;

    case S2C_ESCAPE_RESULT:         // 탈출 결과 반영
        break;

    default:
        return false;
    }

    return true;
}