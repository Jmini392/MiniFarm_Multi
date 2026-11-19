#include "PCH.h"
#include "Packet.h"
#include "GameWorld.h"

// main.cpp에 정의된 월드를 사용한다.
extern GameWorld g_world;

std::vector<ClientSession*> g_sessions; // 접속 중인 세션 목록
CriticalSection g_sessionCs; // 세션 목록 보호용 임계영역

int ClientSession::Recv(char* buf)
{
	// 고정 길이 헤더 받기
	int retval = recv(m_sock, buf, PACKET_HEADER_SIZE, MSG_WAITALL);
	if (retval == 0) return 0;
	if (retval != PACKET_HEADER_SIZE) return -1;

	PacketHeader header;
	memcpy(&header, buf, sizeof(header));
	if (header.size < PACKET_HEADER_SIZE) return -1;

	// 헤더가 알려 준 크기만큼 가변 길이 본문 받기
	int bodySize = header.size - PACKET_HEADER_SIZE;
	if (bodySize > 0) {
		retval = recv(m_sock, buf + PACKET_HEADER_SIZE, bodySize, MSG_WAITALL);
		if (retval == 0) return 0;
		if (retval != bodySize) return -1;
	}
	return header.size;
}

bool ClientSession::Send(const char* packet)
{
	PacketHeader header;
	memcpy(&header, packet, sizeof(header));
	if (header.size < PACKET_HEADER_SIZE) return false;

	// 다른 스레드의 패킷과 섞이지 않도록 송신 락을 잡고 보낸다
	m_sendCs.Lock();
	int retval = send(m_sock, packet, header.size, 0);
	m_sendCs.Unlock();

	return retval == header.size;
}

// 세션 ID로 접속 중인 클라이언트를 찾아 패킷을 전송한다.
bool SendToClient(int sessionId, const char* packet)
{
	bool result = false;

	// 세션을 조회
	g_sessionCs.Lock();
	for (ClientSession* session : g_sessions) {
		if (session->GetId() == sessionId) {
			// 해당 세션의 송신 함수를 통해 패킷 전체를 전송한다.ㄴ
			result = session->Send(packet);   
			break;
		}
	}
	g_sessionCs.Unlock();

	return result;
}

// 초기 동기화가 끝난 모든 클라이언트에게 패킷을 전송한다.
void SendToAll(const char* packet)
{
	// 로그인 전이거나 초기 상태를 받는 중인 접속자는 대상에서 빠진다.
	g_world.Lock();
	const std::vector<int> receivers = g_world.GetFieldSyncPlayers();
	g_world.Unlock();

	// 월드 잠금을 해제한 뒤 송신한다.
	for (int receiverId : receivers) {
		if (!SendToClient(receiverId, packet))
			printf("[전체 전송 실패] 대상 ID=%d\n", receiverId);
	}
}
