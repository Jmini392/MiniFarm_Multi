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
	C2S_DEBUG_WALLET_CHANGE = 20,
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
	S2C_INVENTORY_UPDATE = 18,
	S2C_WALLET_BALANCE = 19,
	S2C_FIELD_INTERACT_RESULT = 21,
};

enum class DebugWalletAction : std::uint8_t
{
    Add = 1,
    Spend = 2,
};

enum class FieldInteractStatus : std::uint8_t
{
    Success = 0,
    PlayerNotReady = 1,
    ServerFarmingDisabled = 2,
    UnknownField = 3,
    TooFar = 4,
    UnsupportedTool = 5,
    NotApplicable = 6,
    NoSeed = 7,
    InvalidCropData = 8,
    NotMature = 9,
    InventoryLimitExceeded = 10,
    InvalidCropState = 11,
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

struct C2S_MOVE_OBJECT_PACKET
{
    PacketHeader header;
    float x;
    float y;
    float z;
    float yaw;

    std::uint8_t isMoving;   // 0: 정지, 1: 이동
    std::uint8_t isAirborne; // 0: 지상, 1: 공중
};

struct C2S_EQUIP_PACKET
{
    PacketHeader header;
    std::int32_t itemId; // 선택한 도구 또는 씨앗 ID. Empty(-1)도 표현한다.
};

struct C2S_INTERACT_FIELD_PACKET
{
    PacketHeader header;
    std::int32_t objectId;
};

struct C2S_DEBUG_WALLET_CHANGE_PACKET
{
    PacketHeader header;
    DebugWalletAction action;
};

struct S2C_WALLET_BALANCE_PACKET
{
    PacketHeader header;
    std::int32_t balance;
};

struct InventoryCounts
{
    std::int32_t carrotSeedCount;
    std::int32_t cabbageSeedCount;
    std::int32_t carrotCount;
    std::int32_t cabbageCount;
};

struct S2C_LOGIN_INFO_PACKET
{
    PacketHeader header;
    std::int32_t playerId;
    float x;
    float y;
    float z;
    float yaw;
    InventoryCounts counts;
};

struct S2C_INVENTORY_UPDATE_PACKET
{
    PacketHeader header;
    InventoryCounts counts;
};

struct S2C_ADD_OBJECT_PACKET
{
    PacketHeader header;
    std::int32_t objectId;       // 서버와 클라이언트가 같은 객체에 같은 번호를 쓴다
    unsigned char objectType;    // 객체 종류: 0 밭(Field), 1 집(House), 2 보트(Boat)
    float pos[3];
    float rot[3];
    float scale[3];
};

struct S2C_MOVE_OBJECT_PACKET
{
    PacketHeader header;
    std::int32_t objectId;
    float x;
    float y;
    float z;
    float yaw;

    std::uint8_t isMoving;
    std::uint8_t isAirborne;
};

struct S2C_EQUIP_OBJECT_PACKET
{
    PacketHeader header;
    std::int32_t objectId;
    std::int32_t itemId; // 도구, 씨앗 또는 Empty(-1)
};

struct S2C_FIELD_UPDATE_PACKET
{
    PacketHeader header;
    std::int32_t objectId;
    std::int32_t cropId; // Empty(-1), Tilled(0), Carrot(1), Cabbage(2)
    std::int32_t level;
    float water;
    float growTime;
    float requiredGrowTime;
};

// Only sent when a field interaction is rejected.
struct S2C_FIELD_INTERACT_RESULT_PACKET
{
    PacketHeader header;
    std::int32_t objectId;
    FieldInteractStatus reason;
};

struct S2C_REMOVE_OBJECT_PACKET
{
    PacketHeader header;
    std::int32_t objectId;
};

struct S2C_TIME_SYNC_PACKET
{
    PacketHeader header;
    float dayTime;
};

#pragma pack(pop)

constexpr unsigned char PACKET_HEADER_SIZE = sizeof(PacketHeader);