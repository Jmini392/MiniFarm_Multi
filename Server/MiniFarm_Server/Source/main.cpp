#include "Common.h"
#include "RecvPacket.h"
#include "SendPacket.h"
#include "GameWorld.h"
#include <vector>
#include <algorithm>

// 접속한 클라이언트 한 명을 나타내는 세션
// 클라이언트 스레드가 하나씩 맡고, 스레드가 끝날 때 delete 한다
class ClientSession {
public:
	ClientSession(int id, SOCKET sock, const sockaddr_in& addr) : m_id(id), m_sock(sock) {
		inet_ntop(AF_INET, &addr.sin_addr, m_ip, sizeof(m_ip)); // 클라이언트 IP주소를 문자열로 변환
		m_port = ntohs(addr.sin_port); // 클라이언트 포트번호를 호스트 바이트 순서로 변환
		InitializeCriticalSection(&m_sendCs);
	}
	~ClientSession() {
		closesocket(m_sock); // 세션이 사라지면 소켓도 닫는다
		DeleteCriticalSection(&m_sendCs);
	}

	// 소켓과 임계영역을 가진 객체이므로 복사하지 않는다
	ClientSession(const ClientSession&) = delete;
	ClientSession& operator=(const ClientSession&) = delete;

	// 이 클라이언트에게 패킷을 보낸다
	// 클라이언트 스레드와 Update 스레드가 같은 소켓으로 동시에 보낼 수 있으므로 송신 락을 잡는다
	bool Send(const char* packet) {
		EnterCriticalSection(&m_sendCs);
		bool ok = SendPacket(m_sock, packet);
		LeaveCriticalSection(&m_sendCs);
		return ok;
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
	CRITICAL_SECTION m_sendCs; // 송신 보호용 임계영역
};

// 모든 스레드가 함께 쓰는 데이터
GameWorld g_world; // 게임 월드 (Lock/Unlock으로 보호)
std::vector<ClientSession*> g_sessions; // 접속 중인 세션 목록
CRITICAL_SECTION g_sessionCs; // 세션 목록 보호용 임계영역

// 클라이언트 스레드: 클라이언트 한 명의 패킷을 연결이 끊길 때까지 받는다
DWORD WINAPI ClientThread(LPVOID arg) {
	ClientSession* session = (ClientSession*)arg;
	char buf[MAX_PACKET_SIZE]; // 받은 패킷

	while (1) {
		int size = RecvPacket(session->GetSocket(), buf);
		if (size <= 0) break; // 0이면 연결 종료, -1이면 오류

		// 패킷의 헤더에서 타입과 크기를 꺼내 출력
		PacketHeader header;
		memcpy(&header, buf, sizeof(header));
		printf("[패킷] %s: 타입 %d, 크기 %d\n", session->GetIp(), header.type, header.size);
	}

	// 세션 목록에서 빼고, 월드에서 플레이어를 제거한다
	EnterCriticalSection(&g_sessionCs);
	g_sessions.erase(std::remove(g_sessions.begin(), g_sessions.end(), session), g_sessions.end());
	int count = (int)g_sessions.size();
	LeaveCriticalSection(&g_sessionCs);

	g_world.Lock();
	g_world.RemovePlayer(session->GetId());
	g_world.Unlock();

	printf("[해제] %s (접속 %d명)\n", session->GetIp(), count);
	delete session; // 소켓도 함께 닫힌다
	return 0;
}

// Update 스레드: 일정 간격마다 월드를 갱신한다 (작물 성장 등)
DWORD WINAPI UpdateThread(LPVOID arg) {
	const DWORD UPDATE_INTERVAL = 100; // 갱신 간격(ms)
	DWORD last = GetTickCount();

	while (1) {
		Sleep(UPDATE_INTERVAL);
		DWORD now = GetTickCount();
		int dt = (int)(now - last); // 지난 시간(ms)
		last = now;

		g_world.Lock();
		g_world.Update(dt);
		g_world.Unlock();
	}
	return 0;
}

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
	if (!g_world.Init("..\\..\\Client\\MiniFarm\\Data\\crops.csv")) return 1;
	printf("[월드] 작물 %d종 로딩\n", (int)g_world.GetCropTable().GetCount());

	InitializeCriticalSection(&g_sessionCs);

	// Update 스레드 시작 (끝나기를 기다리지 않으므로 핸들은 바로 닫는다)
	HANDLE hThread = CreateThread(NULL, 0, UpdateThread, NULL, 0, NULL);
	if (hThread == NULL) return 1;
	CloseHandle(hThread);

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
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
		ClientSession* session = new ClientSession(nextId++, client_sock, clientaddr);

		g_world.Lock();
		g_world.AddPlayer(session->GetId()); // 월드에 플레이어 추가
		g_world.Unlock();

		EnterCriticalSection(&g_sessionCs);
		g_sessions.push_back(session);
		int count = (int)g_sessions.size();
		LeaveCriticalSection(&g_sessionCs);

		printf("[접속] %s (접속 %d명)\n", session->GetIp(), count);

		// 클라이언트 스레드 시작 (세션은 스레드가 끝날 때 정리한다)
		hThread = CreateThread(NULL, 0, ClientThread, session, 0, NULL);
		if (hThread == NULL) {
			// 스레드를 못 만들면 접속을 정리한다
			EnterCriticalSection(&g_sessionCs);
			g_sessions.erase(std::remove(g_sessions.begin(), g_sessions.end(), session), g_sessions.end());
			LeaveCriticalSection(&g_sessionCs);
			g_world.Lock();
			g_world.RemovePlayer(session->GetId());
			g_world.Unlock();
			delete session;
		}
		else {
			CloseHandle(hThread);
		}
	}

	// 소켓 닫기
	closesocket(listen_sock);
	DeleteCriticalSection(&g_sessionCs);
	// 윈속 종료
	WSACleanup();
	return 0;
}
