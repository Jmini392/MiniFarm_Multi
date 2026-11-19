#pragma once

// 접속한 클라이언트 한 명의 세션. 클라이언트 스레드가 맡아 끝날 때 delete 하며, 패킷은 Recv/Send로만 주고받는다
class ClientSession {
public:
	ClientSession(int id, SOCKET sock, const sockaddr_in& addr) : m_id(id), m_sock(sock) {
		inet_ntop(AF_INET, &addr.sin_addr, m_ip, sizeof(m_ip));
	}
	~ClientSession() {
		closesocket(m_sock);
	}

	// 소켓과 임계영역을 가진 객체이므로 복사하지 않는다
	ClientSession(const ClientSession&) = delete;
	ClientSession& operator=(const ClientSession&) = delete;

	int Recv(char* buf);
	bool Send(const char* packet);

	int GetId() const { return m_id; }
	const char* GetIp() const { return m_ip; }

private:
	int m_id; // 세션 ID
	SOCKET m_sock; // 클라이언트 통신 소켓
	char m_ip[INET_ADDRSTRLEN]; // 클라이언트 IP 주소 문자열
	CriticalSection m_sendCs; // 송신 보호용 임계영역
};

// 모든 스레드가 함께 쓰는 세션 목록
extern std::vector<ClientSession*> g_sessions;
extern CriticalSection g_sessionCs;

// 세션 ID로 접속 중인 클라이언트를 찾아 패킷을 전송한다.
bool SendToClient(int sessionId, const char* packet);

// 초기 동기화가 끝난 모든 클라이언트에게 패킷을 전송한다.
void SendToAll(const char* packet);