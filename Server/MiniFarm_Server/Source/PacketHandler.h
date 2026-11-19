#pragma once

// sessionId 세션이 보낸 패킷(헤더 포함)을 타입에 맞는 처리로 넘긴다. 잘못된 타입이면 false
bool HandlePacket(int sessionId, const char* packet);