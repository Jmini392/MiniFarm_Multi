#pragma once

// 까마귀 상태를 접속한 클라이언트에게 알리는 함수 모음

// 현재 까마귀들을 S2C_ADD_OBJECT(objectType 3)로 보낸다. SendWorldObjects 다음에 호출하며, 하나라도 실패하면 false
bool SendCrows(int sessionId);

// 모든 까마귀의 현재 위치를 S2C_MOVE_OBJECT로 초기 동기화가 끝난 접속자에게 보낸다. Update 스레드가 매 갱신마다 호출한다
void BroadcastCrowMoves();
