#pragma once
#include <cstdint>

// 클라이언트와 서버가 함께 쓰는 프로토콜 정의
// 패킷은 [PacketHeader(고정 2바이트)][본문(가변)] 순서로 전송된다
// size는 헤더를 포함한 패킷 전체 크기이므로 패킷 하나는 최대 255바이트다
// 월드 오브젝트처럼 많은 데이터는 한 패킷에 묶지 않고 오브젝트마다 패킷 하나씩 보낸다

constexpr unsigned short SERVER_PORT = 9000;
constexpr unsigned char MAX_PACKET_SIZE = 255; // 헤더를 포함한 패킷 하나의 최대 크기

enum PacketType : unsigned char {
	PACKET_NONE = 0,
	// Client to Server
	C2S_LOGIN = 1,
	C2S_MOVE_OBJECT = 2,
	C2S_EQUIP = 3,
	C2S_INTERACT_FIELD = 4,
	C2S_SHOP_BUY = 5,
	C2S_SHOP_SELL = 6,
	C2S_BOAT_ESCAPE = 7,
	// Server to Client
	S2C_LOGIN_INFO = 8,
	S2C_ADD_OBJECT = 9,
	S2C_REMOVE_OBJECT = 10,
	S2C_MOVE_OBJECT = 11,
	S2C_EQUIP_OBJECT = 12,
	S2C_FIELD_UPDATE = 13,
	S2C_SHOP_BUY_RESULT = 14,
	S2C_SHOP_SELL_RESULT = 15,
	S2C_TIME_SYNC = 16,
	S2C_ESCAPE_RESULT = 17,
};

#pragma pack(push, 1)
struct PacketHeader {
	unsigned char size;
	unsigned char type;
};

struct C2S_LOGIN_PACKET
{
    PacketHeader header;
};

struct S2C_LOGIN_INFO_PACKET
{
    PacketHeader header;
    std::int32_t playerId;
    float x;
    float y;
    float z;
    float yaw;
};

#pragma pack(pop)

constexpr unsigned char PACKET_HEADER_SIZE = sizeof(PacketHeader);
