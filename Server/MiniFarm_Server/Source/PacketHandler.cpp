#include "PacketHandler.h"
#include "GameWorld.h"
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

            g_world.Unlock();

            // 송신 중에는 월드 잠금을 잡지 않는다.
            if (!SendToClient(sessionId, (const char*)&info))
            {
                printf("[로그인 응답 실패] ID=%d\n", sessionId);
                return false;
            }

            printf("[로그인 완료] ID=%d\n", sessionId);

            return true;
		}
		case C2S_MOVE_OBJECT:    // 이동 처리
		case C2S_EQUIP:          // 장비 변경 처리
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
