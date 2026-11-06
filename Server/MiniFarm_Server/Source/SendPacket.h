#pragma once
#include "Common.h"
#include "../../../Shared/Protocol.h"

// 헤더를 포함한 패킷 전체를 모두 보낸다
// send는 한 번에 다 보내지 못할 수 있으므로, 패킷 전체가 나갈 때까지 반복한다
// packet의 맨 앞은 PacketHeader이고, 보낼 크기는 헤더의 size를 따른다
// 반환값: 모두 보냈으면 true, 오류 또는 잘못된 패킷이면 false
bool SendPacket(SOCKET sock, const char* packet);
