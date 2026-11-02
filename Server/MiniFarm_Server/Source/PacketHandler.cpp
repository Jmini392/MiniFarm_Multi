#include "PacketHandler.h"
#include <cstring>
#include "../../../Shared/Protocol.h"

bool HandlePacket(int sessionId, const char* packet) {
	PacketHeader header;
	memcpy(&header, packet, sizeof(header));

	switch (header.type) {
		case C2S_LOGIN:          // 로그인 처리
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
