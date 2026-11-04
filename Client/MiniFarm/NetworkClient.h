#pragma once

#include <WinSock2.h>

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

private:
    SOCKET m_socket = INVALID_SOCKET;
    bool m_wsaInitialized = false;
    bool m_connected = false;

    RecvBuffer* m_recvBuffer = nullptr;
};