#include "PCH.h"
#include "CrowSync.h"
#include "GameWorld.h"
#include "Packet.h"

// main.cpp에 정의된 월드를 사용한다.
extern GameWorld g_world;

bool SendCrows(int sessionId)
{
	// 까마귀 위치는 Update 스레드가 바꾸므로 월드 잠금을 잡고 복사한 뒤 잠금 없이 보낸다
	g_world.Lock();
	const std::vector<CrowState> crows = g_world.GetCrowSystem().GetCrows();
	g_world.Unlock();

	for (const CrowState& crow : crows) {
		S2C_ADD_OBJECT_PACKET add{};
		add.header.size = (unsigned char)sizeof(add);
		add.header.type = S2C_ADD_OBJECT;
		add.objectId = crow.id;
		add.objectType = 3; // 까마귀
		add.pos[0] = crow.pos.x; add.pos[1] = crow.pos.y; add.pos[2] = crow.pos.z;
		add.rot[1] = crow.yaw;
		add.scale[0] = CROW_SCALE; add.scale[1] = CROW_SCALE; add.scale[2] = CROW_SCALE;

		if (!SendToClient(sessionId, (const char*)&add)) {
			return false;
		}
	}
	return true;
}

void BroadcastCrowMoves()
{
	// 까마귀 위치와 받을 사람을 잠금 안에서 복사하고, 전송은 잠금 없이 한다
	g_world.Lock();
	const std::vector<CrowState> crows = g_world.GetCrowSystem().GetCrows();
	const std::vector<int> receivers = g_world.GetFieldSyncPlayers();
	g_world.Unlock();

	for (const CrowState& crow : crows) {
		S2C_MOVE_OBJECT_PACKET move{};
		move.header.size = (unsigned char)sizeof(move);
		move.header.type = S2C_MOVE_OBJECT;
		move.objectId = crow.id;
		move.x = crow.pos.x;
		move.y = crow.pos.y;
		move.z = crow.pos.z;
		move.yaw = crow.yaw;
		move.isMoving = crow.action != CrowAction::OnField ? 1 : 0; // 밭에 내려앉으면 멈춘다
		move.isAirborne = crow.action != CrowAction::OnField ? 1 : 0;

		for (int receiverId : receivers) {
			SendToClient(receiverId, (const char*)&move);
		}
	}
}
