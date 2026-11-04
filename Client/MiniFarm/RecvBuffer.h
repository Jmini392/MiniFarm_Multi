#pragma once

#include <vector>
#include <queue>

class RecvBuffer
{
public:
    bool Append(const char* data, int size);
    bool PopPacket(std::vector<char>& packet);
    void Clear();

private:
    std::vector<char> m_buffer;
    std::queue<std::vector<char>> m_packets;
};