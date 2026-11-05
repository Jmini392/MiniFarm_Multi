#pragma once

#include <WinSock2.h>
#include <vector>
#include <queue>

class RecvBuffer
{
public:
    RecvBuffer();
    ~RecvBuffer();

    RecvBuffer(const RecvBuffer&) = delete;
    RecvBuffer& operator=(const RecvBuffer&) = delete;

    bool Append(const char* data, int size);
    bool PopPacket(std::vector<char>& packet);
    void Clear();

private:
    std::vector<char> m_buffer;
    std::queue<std::vector<char>> m_packets;

    CRITICAL_SECTION m_cs;
};