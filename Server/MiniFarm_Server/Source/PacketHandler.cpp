#include "PacketHandler.h"
#include "GameWorld.h"
#include "WorldSync.h"
#include <cstring>
#include "../../../Shared/Protocol.h"

// main.cpp에 정의된 월드를 사용한다.
extern GameWorld g_world;

// main.cpp에 구현한 송신 함수를 사용한다.
bool SendToClient(int sessionId, const char* packet);

bool HandlePacket(int sessionId, const char* packet) {
	PacketHeader header;
	memcpy(&header, packet, sizeof(header));

	switch (header.type) {
		case C2S_LOGIN:          // 로그인 처리
		{
			if (header.size != sizeof(C2S_LOGIN_PACKET))
				return false;

            S2C_LOGIN_INFO_PACKET info{};

            info.header.size = (unsigned char)sizeof(info);
            info.header.type = S2C_LOGIN_INFO;

            g_world.Lock();

            // 이미 로그인한 세션의 중복 요청은 거부한다.
            if (g_world.GetPlayer(sessionId) != nullptr)
            {
                g_world.Unlock();
                return false;
            }

            g_world.AddPlayer(sessionId);

            PlayerState* player = g_world.GetPlayer(sessionId);

            if (player == nullptr)
            {
                g_world.Unlock();
                return false;
            }

            const PlayerPosition& pos = player->GetPosition();

            info.playerId = player->GetId();
            info.x = pos.x;
            info.y = pos.y;
            info.z = pos.z;
            info.yaw = player->GetYaw();

            const InventoryCore& inventory = player->GetInventory();
            info.carrotSeedCount = inventory.GetItemCount(EItemID::SeedCarrot);
            info.cabbageSeedCount = inventory.GetItemCount(EItemID::SeedCabbage);
            info.carrotCount = inventory.GetItemCount(EItemID::Carrot);
            info.cabbageCount = inventory.GetItemCount(EItemID::Cabbage);

            g_world.Unlock();

            // 송신 중에는 월드 잠금을 잡지 않는다.
            if (!SendToClient(sessionId, (const char*)&info))
            {
                printf("[로그인 응답 실패] ID=%d\n", sessionId);
                return false;
            }

            // 로그인 응답 뒤에 월드 객체를 보낸다.
            if (!SendWorldObjects(sessionId))
            {
                printf("[객체 전송 실패] ID=%d\n", sessionId);
                return false;
            }

            // 클라이언트에 밭 객체가 생성된 뒤 같은 ID의 상태를 적용한다.
            if (!SendFieldStates(sessionId))
            {
                printf("[밭 상태 전송 실패] ID=%d\n", sessionId);
                return false;
            }

            // 현재 접속자들의 위치·장비를 보내고, 새 플레이어도 기존 접속자에게 알린다.
            if (!SendPlayerStates(sessionId))
            {
                printf("[플레이어 상태 전송 실패] ID=%d\n", sessionId);
                return false;
            }

            printf("[로그인 완료] ID=%d (객체 %d개 전송)\n", sessionId, (int)g_world.GetObjects().size());

            return true;
		}
        case C2S_MOVE_OBJECT:    // 이동 처리
        {
            if (header.size != sizeof(C2S_MOVE_OBJECT_PACKET))
                return false;

            C2S_MOVE_OBJECT_PACKET request{};
            memcpy(&request, packet, sizeof(request));

            if (request.isMoving > 1 || request.isAirborne > 1)
                return false;

            bool success = false;

            g_world.Lock();

            PlayerState* player = g_world.GetPlayer(sessionId);

            if (player != nullptr)
            {
                success = player->SetMovement(request.x,request.y,request.z,request.yaw,
                    request.isMoving != 0, request.isAirborne != 0);
            }

            g_world.Unlock();

            if (!success)
                return false;

            SendPlayerMove(sessionId);

            return true;
        }
		case C2S_EQUIP:          // 장비 변경 처리
		{
			if (header.size != sizeof(C2S_EQUIP_PACKET))
				return false;

			C2S_EQUIP_PACKET request{};
			memcpy(&request, packet, sizeof(request));

			bool success = false;
			bool changed = false;
			g_world.Lock();
			PlayerState* player = g_world.GetPlayer(sessionId);
			if (player != nullptr)
			{
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
		case C2S_INTERACT_FIELD: // 밭 상호작용 처리
		case C2S_SHOP_BUY:       // 구매 처리
		case C2S_SHOP_SELL:      // 판매 처리
		case C2S_BOAT_ESCAPE:    // 탈출 처리
			(void)sessionId;
			return true;
		default:
			return false; // 모르는 타입
	}
}
