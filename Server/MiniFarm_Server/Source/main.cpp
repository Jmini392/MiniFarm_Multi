#include "PCH.h"
#include "Packet.h"
#include "GameWorld.h"
#include "PacketHandler.h"
#include "WorldSync.h"
#include "CrowSync.h"

// 모든 스레드가 함께 쓰는 데이터
GameWorld g_world; // 게임 월드

// 클라이언트 스레드: 클라이언트 한 명의 패킷을 연결이 끊길 때까지 받는다
DWORD WINAPI ClientThread(LPVOID arg) {
	ClientSession* session = (ClientSession*)arg;
	char buf[MAX_PACKET_SIZE]; // 받은 패킷
	bool loggedIn = false; // 로그인 기록

	while (1) {
		int size = session->Recv(buf);

		if (size <= 0) break;

		// 패킷의 헤더에서 타입과 크기를 꺼내 출력
		PacketHeader header;
		memcpy(&header, buf, sizeof(header));

		// 로그인 전에 다른 요청을 보내면 거부
		if (!loggedIn && header.type != C2S_LOGIN) break;
		// 로그인 완료 후 중복 로그인 요청 거부
		if (loggedIn && header.type == C2S_LOGIN) break;

		// 패킷 처리
		if (!HandlePacket(session->GetId(), buf)) {
			printf("[패킷 처리 실패] ID=%d\n", session->GetId());
			break;
		}

		// 처음 패킷이 로그인인지 확인
		if (header.type == C2S_LOGIN) loggedIn = true;
	}

	// 세션 목록에서 빼고, 월드에서 플레이어를 제거한다
	g_sessionCs.Lock();
	g_sessions.erase(std::remove(g_sessions.begin(), g_sessions.end(), session), g_sessions.end());
	int count = (int)g_sessions.size();
	g_sessionCs.Unlock();

	g_world.Lock();
	bool wasInWorld = g_world.GetPlayer(session->GetId()) != nullptr;
	g_world.RemovePlayer(session->GetId());
	g_world.Unlock();

	// 월드에 있던 플레이어라면 남은 클라이언트에게 삭제를 알린다
	if (wasInWorld) SendRemoveObject(session->GetId());

	printf("[해제] %s (접속 %d명)\n", session->GetIp(), count);
	delete session; // 소켓도 함께 닫힌다
	return 0;
}

// Update 스레드: 일정 간격마다 월드를 갱신한다 (작물 성장 등)
DWORD WINAPI UpdateThread(LPVOID arg) {
	const DWORD UPDATE_INTERVAL = 100; // 갱신 간격(ms)
	DWORD last = GetTickCount();

	constexpr int TIME_SYNC_INTERVAL_MS = 1000;
	int timeSyncElapsedMs = 0;

	while (1) {
		Sleep(UPDATE_INTERVAL);
		DWORD now = GetTickCount();
		int dt = (int)(now - last); // 지난 시간(ms)
		last = now;

			g_world.Lock();
			g_world.Update(dt);
			g_world.Unlock();

		// 월드 잠금을 해제한 뒤, 접속 초기화가 끝난 클라이언트에 변경 상태를 보낸다.
		SendChangedFieldStates();
		SendChangedWalletBalances();
		BroadcastCrowMoves();

		timeSyncElapsedMs += dt;
		if (timeSyncElapsedMs >= TIME_SYNC_INTERVAL_MS)
		{
			timeSyncElapsedMs %= TIME_SYNC_INTERVAL_MS;
			BroadcastTimeSync();
		}
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
	if (!g_world.Init("../../Shared/Data/crops.csv")) return 1;
	printf("[월드] 작물 %d종 로딩\n", (int)g_world.GetCropTable().GetCount());
	if (!g_world.LoadObjects("../../Shared/Data/Interactable_props_pos.json")) return 1;
	printf("[월드] 객체 %d개 로딩 (밭 %d개)\n", (int)g_world.GetObjects().size(), g_world.GetFieldCount());

	// 까마귀 세 마리를 맵 여기저기 상공에 만든다. 위치는 플레이어 좌표이며 배회 구역 안이다
	const Vec3f CROW_STARTS[] = {
		{ -3.0f, 15.0f, 7.0f },   // 밭 위쪽
		{ 15.0f, 15.0f, -10.0f }, // 오른쪽 아래
		{ -18.0f, 15.0f, 20.0f }, // 왼쪽 위
	};
	for (const Vec3f& start : CROW_STARTS) {
		g_world.Lock();
		const int crowId = g_world.GetCrowSystem().AddCrow(start);
		g_world.Unlock();
		if (crowId < 0) return 1;
		printf("[월드] 까마귀 생성 ID=%d 위치=(%.1f, %.1f, %.1f)\n", crowId, start.x, start.y, start.z);
	}

	// Update 스레드
	HANDLE hThread = CreateThread(NULL, 0, UpdateThread, NULL, 0, NULL);
	if (hThread == NULL) return 1;
	CloseHandle(hThread);

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	int Id = 1;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트마다 세션 생성
		ClientSession* session = new ClientSession(Id++, client_sock, clientaddr);

		g_sessionCs.Lock();
		g_sessions.push_back(session);
		int count = (int)g_sessions.size();
		g_sessionCs.Unlock();

		printf("[접속] %s (접속 %d명)\n", session->GetIp(), count);

		// 클라이언트 스레드
		hThread = CreateThread(NULL, 0, ClientThread, session, 0, NULL);
		if (hThread == NULL) {
			// 스레드를 못 만드는 경우, 접속을 정리한다.
			g_sessionCs.Lock();
			g_sessions.erase(std::remove(g_sessions.begin(), g_sessions.end(), session), g_sessions.end());
			g_sessionCs.Unlock();
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
	// 윈속 종료
	WSACleanup();
	return 0;
}
