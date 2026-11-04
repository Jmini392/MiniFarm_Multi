#include "PCH.h"
#include "PacketHandler.h"
#include "../../Shared/Protocol.h"

#include <cstring>

bool HandlePacket(const char* packet)
{
    if (packet == nullptr)
        return false;

    PacketHeader header;
    memcpy(&header, packet, sizeof(header));

    switch (header.type)
    {
    case S2C_LOGIN_INFO:
        // 내 플레이어 초기화
        break;

    case S2C_ADD_OBJECT:
        // 월드 객체 생성
        break;

    case S2C_REMOVE_OBJECT:
        // 월드 객체 삭제
        break;

    case S2C_MOVE_OBJECT:
        // 객체 위치와 방향 반영
        break;

    case S2C_EQUIP_OBJECT:
        // 장착 도구 반영
        break;

    case S2C_FIELD_UPDATE:
        // 밭과 작물 상태 반영
        break;

    case S2C_SHOP_BUY_RESULT:
        // 구매 결과 반영
        break;

    case S2C_SHOP_SELL_RESULT:
        // 판매 결과 반영
        break;

    case S2C_TIME_SYNC:
        // 게임 시간 반영
        break;

    case S2C_ESCAPE_RESULT:
        // 탈출 결과 반영
        break;

    default:
        return false;
    }

    return true;
}