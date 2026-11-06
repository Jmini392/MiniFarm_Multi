#include "SendPacket.h"

bool SendPacket(SOCKET sock, const char* packet)
{
	PacketHeader header;
	memcpy(&header, packet, sizeof(header));
	if (header.size < PACKET_HEADER_SIZE) {
		return false; // 잘못된 크기
	}

	// 패킷 전체가 나갈 때까지 보낸다
	int sent = 0;
	while (sent < header.size) {
		int retval = send(sock, packet + sent, header.size - sent, 0);
		if (retval == SOCKET_ERROR) {
			return false;
		}
		sent += retval;
	}
	return true;
}
