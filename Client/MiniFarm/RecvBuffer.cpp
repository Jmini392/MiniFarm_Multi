#include "PCH.h"
#include "RecvBuffer.h"
#include "../../Shared/Protocol.h"

#include <cstring>

bool RecvBuffer::Append(const char* data, int size)
{
    if (data == nullptr || size <= 0)
        return false;

    // 이번에 받은 데이터를 기존 데이터 뒤에 붙인다.
    m_buffer.insert(m_buffer.end(), data, data + size);

    while (m_buffer.size() >= PACKET_HEADER_SIZE)
    {
        PacketHeader header;
        memcpy(&header, m_buffer.data(), sizeof(header));

        // 현재 size는 1바이트이므로 최대 255다.
        if (header.size < PACKET_HEADER_SIZE)
        {
            m_buffer.clear();
            return false;
        }

        // 아직 패킷 전체가 도착하지 않았다.
        if (m_buffer.size() < header.size)
            break;

        // 완성된 패킷을 큐에 저장한다.
        std::vector<char> packet(m_buffer.begin(), m_buffer.begin() + header.size);

        m_packets.push(packet);

        //LOG("패킷 완성: 크기=%u, 타입=%u",
        //    (unsigned int)header.size,
        //    (unsigned int)header.type);

        // 큐에 넣은 부분만 제거한다.
        m_buffer.erase(m_buffer.begin(),m_buffer.begin() + header.size);
    }

    return true;
}

bool RecvBuffer::PopPacket(std::vector<char>& packet)
{
    if (m_packets.empty())
    {
        packet.clear();
        return false;
    }

    packet = m_packets.front();
    m_packets.pop();

    return true;
}

void RecvBuffer::Clear()
{
    m_buffer.clear();

    while (!m_packets.empty())
    {
        m_packets.pop();
    }
}