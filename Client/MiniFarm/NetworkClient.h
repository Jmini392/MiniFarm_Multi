#pragma once

#include <WinSock2.h>
#include <atomic>
#include "../../Common/GameTypes.h"

class RecvBuffer;

class NetworkClient
{
public:
    NetworkClient();
    ~NetworkClient();

    // 소켓 소유권이 복사되지 않도록 막는다.
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    bool Connect(const char* ip, unsigned short port);
    void Update();
    void Disconnect();

    bool SendLoginRequest();        // 플레이어를 생성하고 초기 정보 얻기
    bool SendEquipRequest(EItemID item);

private:
    bool SendAll(const char* data, int size);

    static DWORD WINAPI ReceiveThread(LPVOID parameter);
    void ReceiveLoop();

    SOCKET m_socket = INVALID_SOCKET;
    bool m_wsaInitialized = false;
    bool m_connected = false;

    RecvBuffer* m_recvBuffer = nullptr;

    HANDLE m_recvThread = NULL;
    std::atomic<bool> m_receiving{ false };
};