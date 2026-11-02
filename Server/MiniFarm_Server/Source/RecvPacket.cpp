#include "RecvPacket.h"

int RecvPacket(SOCKET sock, char* buf)
{
	// 고정 길이 헤더 받기
	int retval = recv(sock, buf, PACKET_HEADER_SIZE, MSG_WAITALL);
	if (retval == 0) {
		return 0; // 연결 종료
	}
	if (retval != PACKET_HEADER_SIZE) {
		return -1;
	}

	PacketHeader header;
	memcpy(&header, buf, sizeof(header));
	if (header.size < PACKET_HEADER_SIZE){
		return -1; // 잘못된 크기
	}

	// 헤더가 알려 준 크기만큼 가변 길이 본문 받기
	int bodySize = header.size - PACKET_HEADER_SIZE;
	if (bodySize > 0) {
		retval = recv(sock, buf + PACKET_HEADER_SIZE, bodySize, MSG_WAITALL);
		if (retval == 0) {
			return 0;
		}
		if (retval != bodySize) {
			return -1;
		}
	}
	return header.size;
}
