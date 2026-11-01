#include "PCH.h"
#include "NetworkClient.h"

#pragma comment(lib, "Ws2_32.lib")

NetworkClient::~NetworkClient()
{
    Disconnect();
}

bool NetworkClient::Connect(const char* ip, unsigned short port)
{
    if (ip == nullptr || port == 0)
    {
        LOG_E("잘못된 서버 주소, 포트");
        return false;
    }

    int result;

    // Winsock 초기화
    WSADATA wsa{};
    result = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (result != 0)
    {
        return false;
    }

    m_wsaInitialized = true;

    // TCP 소켓 생성
    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_socket == INVALID_SOCKET)
    {
        LOG_E("socket()");
        Disconnect();
        return false;
    }

    // connect
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(port);
    result = connect(m_socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (result == SOCKET_ERROR)
    {
        LOG_E("connect()");
        Disconnect();
        return false;
    }

    m_connected = true;
    LOG("클라이언트 접속: IP 주소=%s, 포트번호=%u", ip, (unsigned int)port);
    return true;
}

void NetworkClient::Disconnect()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    m_connected = false;

    if (m_wsaInitialized)
    {
        WSACleanup();
        m_wsaInitialized = false;
    }
}