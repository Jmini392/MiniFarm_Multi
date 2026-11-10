#include "PCH.h"
#include "NetworkClient.h"
#include "RecvBuffer.h"
#include "PacketHandler.h"
#include "../../Shared/Protocol.h"

#pragma comment(lib, "Ws2_32.lib")

#define DATASIZE    4096

NetworkClient::NetworkClient()
{
    m_recvBuffer = new RecvBuffer;
}

NetworkClient::~NetworkClient()
{
    Disconnect();
    delete m_recvBuffer;
}

bool NetworkClient::Connect(const char* ip, unsigned short port)
{
    if (ip == nullptr || port == 0)
    {
        LOG_E("잘못된 서버 주소, 포트");
        return false;
    }

    // 중복 연결을 막는 검사
    if (m_socket != INVALID_SOCKET)
        return false;

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

    m_receiving = true;
    m_recvThread = CreateThread(NULL, 0, ReceiveThread, this, 0, NULL);
    if (m_recvThread == NULL)
    {
        LOG_E("수신 스레드 생성 실패");
        Disconnect();
        return false;
    }

    m_connected = true;
    LOG("서버 접속 및 수신 스레드 생성 성공");
    return true;
}

// 큐 처리만
void NetworkClient::Update()
{
    if (m_socket == INVALID_SOCKET)
        return;

    std::vector<char> packet;

    while (m_recvBuffer->PopPacket(packet))
    {
        if (!HandlePacket(packet.data()))
        {
            LOG_E("패킷 처리 실패");
            Disconnect();
            return;
        }
    }

    // 수신이 끝났다면 메인 스레드에서 정리한다.
    if (!m_receiving)
        Disconnect();
}

void NetworkClient::Disconnect()
{
    m_receiving = false;

    // 블로킹 recv()에서 기다리는 스레드를 깨운다.
    if (m_socket != INVALID_SOCKET)
        shutdown(m_socket, SD_BOTH);

    // 수신 스레드가 끝날 때까지 기다린다.
    if (m_recvThread != NULL)
    {
        WaitForSingleObject(m_recvThread, INFINITE);
        CloseHandle(m_recvThread);
        m_recvThread = NULL;
    }

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

    m_recvBuffer->Clear();
}

bool NetworkClient::SendLoginRequest()
{
    if (m_socket == INVALID_SOCKET)
        return false;

    C2S_LOGIN_PACKET packet{};
    packet.header.size = (unsigned char)sizeof(packet);
    packet.header.type = C2S_LOGIN;

    if (!SendAll(reinterpret_cast<const char*>(&packet), static_cast<int>(sizeof(packet))))
        return false;

    LOG("C2S_LOGIN 전송 완료");
    return true;
}

bool NetworkClient::SendMoveRequest(float x, float y, float z,float yaw, bool isMoving, bool isAirborne)
{
    if (m_socket == INVALID_SOCKET)
        return false;

    C2S_MOVE_OBJECT_PACKET packet{};
    packet.header.size = static_cast<unsigned char>(sizeof(packet));
    packet.header.type = C2S_MOVE_OBJECT;

    packet.x = x;
    packet.y = y;
    packet.z = z;
    packet.yaw = yaw;
    packet.isMoving = isMoving ? 1 : 0;
    packet.isAirborne = isAirborne ? 1 : 0;

    if (!SendAll(reinterpret_cast<const char*>(&packet), static_cast<int>(sizeof(packet))))
    {
        LOG_E("이동 패킷 전송 실패");
        Disconnect();
        return false;
    }

    return true;
}

bool NetworkClient::SendEquipRequest(EItemID item)
{
    C2S_EQUIP_PACKET packet{};
    packet.header.size = (unsigned char)sizeof(packet);
    packet.header.type = C2S_EQUIP;
    packet.itemId = static_cast<std::int32_t>(item);

    return SendAll(reinterpret_cast<const char*>(&packet), static_cast<int>(sizeof(packet)));
}

bool NetworkClient::SendAll(const char* data, int size)
{
    if (m_socket == INVALID_SOCKET || !m_connected || !m_receiving
        || data == nullptr || size <= 0)
        return false;

    int totalSent = 0;
    while (totalSent < size)
    {
        int sent = send(m_socket, data + totalSent, size - totalSent, 0);
        if (sent == SOCKET_ERROR || sent == 0)
            return false;

        totalSent += sent;
    }
    return true;
}

DWORD WINAPI NetworkClient::ReceiveThread(LPVOID parameter)
{
    NetworkClient* client = (NetworkClient*)parameter;

    client->ReceiveLoop();

    return 0;
}

void NetworkClient::ReceiveLoop()
{
    while (m_receiving)
    {
        char data[DATASIZE];

        int result;
        result = recv(m_socket, data, sizeof(data), 0);

        // Disconnect()가 종료를 요청했다면 끝낸다.
        if (!m_receiving)
            break;

        if (result == SOCKET_ERROR)
        {
            LOG_E("수신 오류: %d", WSAGetLastError());
            break;
        }
        else if (result == 0)
        {
            LOG("서버 연결 종료");
            break;
        }

        if (!m_recvBuffer->Append(data, result))
        {
            LOG_E("잘못된 패킷 크기");
            break;
        }
    }

    m_receiving = false;
}