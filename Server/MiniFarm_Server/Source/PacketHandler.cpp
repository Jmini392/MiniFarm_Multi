#include "PCH.h"
#include "PacketHandler.h"
#include "GameWorld.h"
#include "WorldSync.h"
#include "CrowSync.h"
#include "Packet.h"

// main.cpp에 정의된 월드를 사용한다.
extern GameWorld g_world;

namespace {
    InventoryCounts ReadInventoryCounts(const InventoryCore& inventory) {
        return {
            inventory.GetItemCount(EItemID::SeedCarrot),
            inventory.GetItemCount(EItemID::SeedCabbage),
            inventory.GetItemCount(EItemID::Carrot),
            inventory.GetItemCount(EItemID::Cabbage)
        };
    }

    bool SameInventoryCounts(const InventoryCounts& a, const InventoryCounts& b) {
        return a.carrotSeedCount == b.carrotSeedCount 
            && a.cabbageSeedCount == b.cabbageSeedCount
            && a.carrotCount == b.carrotCount 
            && a.cabbageCount == b.cabbageCount;
    }
}

bool HandlePacket(int sessionId, const char* packet) {
	PacketHeader header;
	memcpy(&header, packet, sizeof(header));

	switch (header.type) {
		case C2S_LOGIN:          // 로그인 처리
		{
			if (header.size != sizeof(C2S_LOGIN_PACKET)) return false;

            S2C_LOGIN_INFO_PACKET info{};

            info.header.size = (unsigned char)sizeof(info);
            info.header.type = S2C_LOGIN_INFO;

            g_world.Lock();

            // 이미 로그인한 세션의 중복 요청은 거부한다.
            if (g_world.GetPlayer(sessionId) != nullptr) {
                g_world.Unlock();
                return false;
            }

            g_world.AddPlayer(sessionId);

            PlayerState* player = g_world.GetPlayer(sessionId);

            if (player == nullptr) {
                g_world.Unlock();
                return false;
            }

            const PlayerPosition& pos = player->GetPosition();

            info.playerId = player->GetId();
            info.x = pos.x;
            info.y = pos.y;
            info.z = pos.z;
            info.yaw = player->GetYaw();

            info.counts = ReadInventoryCounts(player->GetInventory());

            g_world.Unlock();

            // 송신 중에는 월드 잠금을 잡지 않는다.
            if (!SendToClient(sessionId, (const char*)&info)) {
                printf("[로그인 응답 실패] ID=%d\n", sessionId);
                return false;
            }

            // 로그인 응답 뒤에 월드 객체를 보낸다.
            if (!SendWorldObjects(sessionId)) {
                printf("[객체 전송 실패] ID=%d\n", sessionId);
                return false;
            }

            if (!SendCrows(sessionId)) {
                printf("[까마귀 전송 실패] ID=%d\n", sessionId);
                return false;
            }

            // 클라이언트에 밭 객체가 생성된 뒤 같은 ID의 상태를 적용한다.
            if (!SendFieldStates(sessionId)) {
                printf("[밭 상태 전송 실패] ID=%d\n", sessionId);
                return false;
            }

            if (!SendTimeSync(sessionId))
            {
                printf("[초기 시각 전송 실패] ID=%d\n", sessionId);
                return false;
            }

            // 현재 접속자들의 위치·장비를 보내고, 새 플레이어도 기존 접속자에게 알린다.
            if (!SendPlayerStates(sessionId)) {
                printf("[플레이어 상태 전송 실패] ID=%d\n", sessionId);
                return false;
            }

            printf("[로그인 완료] ID=%d (객체 %d개 전송)\n", sessionId, (int)g_world.GetObjects().size());

            return true;
		}

        case C2S_MOVE_OBJECT:    // 이동 처리
        {
            if (header.size != sizeof(C2S_MOVE_OBJECT_PACKET)) return false;

            C2S_MOVE_OBJECT_PACKET request{};
            memcpy(&request, packet, sizeof(request));

            if (request.isMoving > 1 || request.isAirborne > 1) return false;

            bool success = false;

            g_world.Lock();

            PlayerState* player = g_world.GetPlayer(sessionId);

            if (player != nullptr) {
                success = player->SetMovement(request.x,request.y,request.z,request.yaw,
                    request.isMoving != 0, request.isAirborne != 0);
            }

            g_world.Unlock();

            if (success) SendPlayerMove(sessionId);

            return success;
        }

		case C2S_EQUIP:          // 장비 변경 처리
		{
			if (header.size != sizeof(C2S_EQUIP_PACKET)) return false;

			C2S_EQUIP_PACKET request{};
			memcpy(&request, packet, sizeof(request));

			bool success = false;
			bool changed = false;

			g_world.Lock();
			
            PlayerState* player = g_world.GetPlayer(sessionId);
			
            if (player != nullptr) {
				const EItemID previous = player->GetEquippedTool();
				success = player->SetEquippedTool(static_cast<EItemID>(request.itemId));
				changed = success && previous != player->GetEquippedTool();
			}
			
            g_world.Unlock();

			if (!success)
				return false;
			if (!changed)
				return true;

			// 서버에 저장된 장비를 월드 잠금 없이 다른 접속자에게 알린다.
			SendPlayerEquip(sessionId);

			printf("[장비 변경] ID=%d, itemId=%d\n", sessionId, (int)request.itemId);
			return true;
		}

#ifdef _DEBUG
        case C2S_DEBUG_WALLET_CHANGE:
        {
            if (header.size != sizeof(C2S_DEBUG_WALLET_CHANGE_PACKET))
                return false;

            C2S_DEBUG_WALLET_CHANGE_PACKET request{};
            memcpy(&request, packet, sizeof(request));
            if (request.action != DebugWalletAction::Add && request.action != DebugWalletAction::Spend)
                return false;

            g_world.Lock();
            const PlayerState* player = g_world.GetPlayer(sessionId);
            if (player == nullptr || !player->IsFieldSyncReady())
            {
                g_world.Unlock();
                return false;
            }

            constexpr int DEBUG_WALLET_AMOUNT = 10000;
            SharedWallet& wallet = g_world.GetWallet();
            const int before = wallet.GetMoney();
            const bool added = request.action == DebugWalletAction::Add;
            const bool success = added ? wallet.AddMoney(DEBUG_WALLET_AMOUNT) : wallet.SpendMoney(DEBUG_WALLET_AMOUNT);
            const int after = wallet.GetMoney();
            g_world.Unlock();

            printf("[Wallet debug] playerId=%d, action=%s, success=%d, balance=%d->%d\n",
                sessionId, added ? "add" : "spend", static_cast<int>(success), before, after);
            return true;
        }
#endif

        case C2S_INTERACT_FIELD:    // 밭 상호작용 처리
        {
            if (header.size != sizeof(C2S_INTERACT_FIELD_PACKET)) return false;

            C2S_INTERACT_FIELD_PACKET request{};
            memcpy(&request, packet, sizeof(request));

            // 요청 판정과 밭·개인 인벤토리 변경을 같은 잠금 안에서 처리한다.
            S2C_INVENTORY_UPDATE_PACKET update{};
            bool inventoryChanged = false;

            g_world.Lock();
            
            const InventoryCore* inventory = g_world.GetInventory(sessionId);
            const InventoryCounts before = inventory != nullptr ? ReadInventoryCounts(*inventory) : InventoryCounts{};
            const FieldInteractResult result = g_world.InteractField(sessionId, request.objectId);
            
            if (result.status == FieldInteractStatus::Success && inventory != nullptr) {
                update.counts = ReadInventoryCounts(*inventory);
                inventoryChanged = !SameInventoryCounts(before, update.counts);
            }

            g_world.Unlock();

            // 형식은 정상인 행동 실패와, 준비되지 않은 요청자를 구분한다.
            if (result.status == FieldInteractStatus::PlayerNotReady) return false;

            // result는 사본이므로 월드 잠금 없이 로그와 디버그에 사용한다.
            printf("[Field action] playerId=%d, objectId=%d, itemId=%d, result=%s, "
                "crop=%d->%d, water=%.1f->%.1f, seed=%d->%d, distanceSquared=%.3f\n",
                sessionId, static_cast<int>(request.objectId), static_cast<int>(result.equippedTool),
                GetFieldInteractStatusName(result.status),
                static_cast<int>(result.before.id), static_cast<int>(result.after.id),
                result.before.water, result.after.water,
                result.seedCountBefore, result.seedCountAfter, result.distanceSquared);

            if (result.status != FieldInteractStatus::Success) {
                S2C_FIELD_INTERACT_RESULT_PACKET rejection{};
                rejection.header.size = static_cast<unsigned char>(sizeof(rejection));
                rejection.header.type = S2C_FIELD_INTERACT_RESULT;
                rejection.objectId = request.objectId;
                rejection.reason = result.status;
                if (!SendToClient(sessionId, reinterpret_cast<const char*>(&rejection))) {
                    printf("[Field rejection send failed] playerId=%d, objectId=%d\n",
                        sessionId, static_cast<int>(rejection.objectId));
                    return false;
                }
                return true;
            }

            // 요청한 플레이어만 자신의 인벤토리를 확인할 수 있습니다.
            if (inventoryChanged) {
                update.header.size = (unsigned char)sizeof(update);
                update.header.type = S2C_INVENTORY_UPDATE;
                if (!SendToClient(sessionId, (const char*)&update)) {
                    printf("[Inventory update send failed] playerId=%d\n", sessionId);
                    return false;
                }
            }
            return true;
        }

		case C2S_SHOP_BUY:       // 구매 처리
		case C2S_SHOP_SELL:      // 판매 처리
		case C2S_BOAT_ESCAPE:    // 탈출 처리
			(void)sessionId;
			return true;
		default:
			return false; // 모르는 타입
	}
}
