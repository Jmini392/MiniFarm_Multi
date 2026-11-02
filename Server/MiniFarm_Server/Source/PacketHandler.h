#pragma once

// 받은 패킷의 타입을 보고 알맞은 처리로 넘긴다
// sessionId: 패킷을 보낸 세션, packet: 헤더를 포함한 패킷 전체
// 잘못된 타입이면 false를 반환한다
bool HandlePacket(int sessionId, const char* packet);