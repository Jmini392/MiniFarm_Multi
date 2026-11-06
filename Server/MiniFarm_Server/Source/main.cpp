#include "Common.h"
#include "RecvPacket.h"
#include "SendPacket.h"
#include "GameWorld.h"
#include <vector>

// 접속한 클라이언트 한 명을 나타내는 세션
class ClientSession {
public:
	ClientSession(int id, SOCKET sock, const sockaddr_in& addr) : m_id(id), m_sock(sock) {
		inet_ntop(AF_INET, &addr.sin_addr, m_ip, sizeof(m_ip)); // 클라이언트 IP주소를 문자열로 변환
		m_port = ntohs(addr.sin_port); // 클라이언트 포트번호를 호스트 바이트 순서로 변환
	}
	~ClientSession() { if (m_sock != INVALID_SOCKET) closesocket(m_sock); } // 세션이 사라지면 소켓도 닫는다

	// 소켓을 가진 객체이므로 복사는 막고, 소켓 소유권을 넘기는 이동만 허용한다
	ClientSession(const ClientSession&) = delete;
	ClientSession& operator=(const ClientSession&) = delete;
	ClientSession(ClientSession&& other) noexcept : m_id(other.m_id), m_sock(other.m_sock), m_port(other.m_port) {
		memcpy(m_ip, other.m_ip, sizeof(m_ip));
		other.m_sock = INVALID_SOCKET; // 넘겨준 쪽은 소켓을 닫지 않도록 비운다
	}
	ClientSession& operator=(ClientSession&& other) noexcept {
		if (this != &other) {
			if (m_sock != INVALID_SOCKET) closesocket(m_sock);
			m_id = other.m_id;
			m_sock = other.m_sock;
			m_port = other.m_port;
			memcpy(m_ip, other.m_ip, sizeof(m_ip));
			other.m_sock = INVALID_SOCKET;
		}
		return *this;
	}

	int GetId() const { return m_id; }
	SOCKET GetSocket() const { return m_sock; }
	const char* GetIp() const { return m_ip; }
	int GetPort() const { return m_port; }

private:
	int m_id; // 세션 ID
	SOCKET m_sock; // 클라이언트 통신 소켓
	char m_ip[INET_ADDRSTRLEN]; // 클라이언트 IP 주소 문자열
	int m_port; // 클라이언트 포트번호
};

int main(int argc, char* argv[]) {
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 게임 월드 준비 (Visual Studio에서 실행하면 작업 디렉터리가 프로젝트 폴더이다)
	GameWorld world;
	if (!world.Init("..\\..\\Client\\MiniFarm\\Data\\crops.csv")) return 1;
	printf("[월드] 작물 %d종 로딩\n", (int)world.GetCropTable().GetCount());

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[MAX_PACKET_SIZE]; // 받은 패킷

	std::vector<ClientSession> sessions; // 접속 중인 세션 목록
	int nextId = 1; // 다음에 부여할 세션 ID

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트마다 세션 생성
		sessions.push_back(ClientSession(nextId++, client_sock, clientaddr));
		const ClientSession& session = sessions.back();
		printf("[접속] %s\n", session.GetIp());
		world.AddPlayer(session.GetId()); // 월드에 플레이어 추가

		// 이 클라이언트가 연결을 끊을 때까지 패킷을 받는다 (끊기면 다음 접속을 기다린다)
		// 스레드는 아직 쓰지 않으므로 한 번에 한 명의 클라이언트만 처리한다
		while (1) {
			int size = RecvPacket(session.GetSocket(), buf);
			if (size <= 0) { // 0이면 연결 종료, -1이면 오류
				printf("[해제] %s\n", session.GetIp());
				world.RemovePlayer(session.GetId()); // 월드에서 플레이어 제거
				break;
			}

			// 패킷의 헤더에서 타입과 크기를 꺼내 출력
			PacketHeader header;
			memcpy(&header, buf, sizeof(header));
			printf("[패킷] %s: 타입 %d, 크기 %d\n", session.GetIp(), header.type, header.size);
		}
	}

	// 소켓 닫기
	closesocket(listen_sock);
	// 윈속 종료
	WSACleanup();
	return 0;
}