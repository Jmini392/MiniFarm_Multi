#include "PCH.h"
#include "PacketHandler.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "IngameScene.h"
#include "../../Shared/Protocol.h"

#include <cstring>

namespace
{
    const char* GetFieldRejectionMessage(FieldInteractStatus reason)
    {
        switch (reason)
        {
        case FieldInteractStatus::ServerFarmingDisabled:
            return "ServerFarmingDisabled (Server farming is disabled.)";
        case FieldInteractStatus::UnknownField:
            return "UnknownField (The field does not exist.)";
        case FieldInteractStatus::TooFar:
            return "TooFar (Move closer to the field.)";
        case FieldInteractStatus::UnsupportedTool:
            return "UnsupportedTool (This tool cannot be used on the field.)";
        case FieldInteractStatus::NotApplicable:
            return "NotApplicable (This tool cannot be used in the field's current state.)";
        case FieldInteractStatus::NoSeed:
            return "NoSeed (No seeds left.)";
        case FieldInteractStatus::InvalidCropData:
            return "InvalidCropData (Crop data is unavailable.)";
        case FieldInteractStatus::NotMature:
            return "NotMature (The crop is not ready to harvest.)";
        case FieldInteractStatus::InventoryLimitExceeded:
            return "InventoryLimitExceeded (Item limit reached.)";
        case FieldInteractStatus::InvalidCropState:
            return "InvalidCropState (Crop state is invalid.)";
        default:
            return nullptr;
        }
    }

    bool AreInventoryCountsValid(const InventoryCounts& counts)
    {
        return counts.carrotSeedCount >= 0 && counts.cabbageSeedCount >= 0
            && counts.carrotCount >= 0 && counts.cabbageCount >= 0;
    }
}

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

        if (!AreInventoryCountsValid(info.counts))
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

    case S2C_REMOVE_OBJECT:
    {
        if (header.size != sizeof(S2C_REMOVE_OBJECT_PACKET))
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());
        if (ingame == nullptr)
            return false;

        S2C_REMOVE_OBJECT_PACKET remove;
        memcpy(&remove, packet, sizeof(remove));

        ingame->RemoveWorldObject(remove.objectId);
        return true;
    }

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
    {
        if (header.size != sizeof(S2C_EQUIP_OBJECT_PACKET))
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());
        if (ingame == nullptr)
            return false;

        S2C_EQUIP_OBJECT_PACKET equip{};
        memcpy(&equip, packet, sizeof(equip));

        return ingame->ApplyEquipObject(equip);
    }

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

    case S2C_WATER_FIELD_EFFECT:
    {
        if (header.size != sizeof(S2C_WATER_FIELD_EFFECT_PACKET))
            return false;

        S2C_WATER_FIELD_EFFECT_PACKET effect{};
        memcpy(&effect, packet, sizeof(effect));
        if (effect.playerId <= 0 || effect.fieldId < 0)
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());
        if (ingame != nullptr)
            ingame->PlayWateringEffect(effect.playerId, effect.fieldId);
        return true;
    }

    case S2C_FIELD_INTERACT_RESULT:
    {
        if (header.size != sizeof(S2C_FIELD_INTERACT_RESULT_PACKET))
            return false;

        S2C_FIELD_INTERACT_RESULT_PACKET rejection{};
        memcpy(&rejection, packet, sizeof(rejection));

        const char* message = GetFieldRejectionMessage(rejection.reason);
        if (message == nullptr)
            return false;

        LOG_D("Field interaction rejected: objectId=%d, %s",
            static_cast<int>(rejection.objectId), message);
        return true;
    }

    case S2C_WALLET_BALANCE:
    {
        if (header.size != sizeof(S2C_WALLET_BALANCE_PACKET))
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());
        if (ingame == nullptr)
            return false;

        S2C_WALLET_BALANCE_PACKET update{};
        memcpy(&update, packet, sizeof(update));
        if (update.balance < 0)
            return false;

        ingame->ApplyServerWalletBalance(update.balance);
        return true;
    }

    case S2C_INVENTORY_UPDATE:
    {
        if (header.size != sizeof(S2C_INVENTORY_UPDATE_PACKET))
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());
        if (ingame == nullptr)
            return false;

        S2C_INVENTORY_UPDATE_PACKET update{};
        memcpy(&update, packet, sizeof(update));
        if (!AreInventoryCountsValid(update.counts))
            return false;

        ingame->ApplyInventoryCounts(update.counts);
        return true;
    }

    case S2C_SHOP_BUY_RESULT:       // 구매 결과 반영
        break;

    case S2C_SHOP_SELL_RESULT:      // 판매 결과 반영
        break;

    case S2C_TIME_SYNC:             // 게임 시간 반영
    {
        if (header.size != sizeof(S2C_TIME_SYNC_PACKET))
            return false;

        S2C_TIME_SYNC_PACKET sync{};
        memcpy(&sync, packet, sizeof(sync));

        if (!std::isfinite(sync.dayTime) || sync.dayTime < 0.f || sync.dayTime >= 24.f)
            return false;

        IngameScene* ingame = dynamic_cast<IngameScene*>(SceneManager::GetScene());
        if (ingame != nullptr)
            ingame->ApplyTimeSync(sync.dayTime);

        return true;
    }

    case S2C_ESCAPE_RESULT:         // 탈출 결과 반영
        break;

    default:
        return false;
    }

    return true;
}