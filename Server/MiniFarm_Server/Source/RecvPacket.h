#pragma once
#include "Common.h"
#include "../../../Shared/Protocol.h"

// 헤더(고정 길이)를 받고, 헤더의 size만큼 본문(가변 길이)을 이어서 받는다
// buf는 MAX_PACKET_SIZE 이상의 크기여야 하고, 성공하면 헤더를 포함한 패킷 전체가 담긴다
// 반환값: 받은 패킷 크기, 0이면 상대가 연결을 끊음, -1이면 오류 또는 잘못된 패킷(연결을 끊어야 함)
int RecvPacket(SOCKET sock, char* buf);
