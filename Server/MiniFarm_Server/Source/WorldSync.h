#pragma once

// 월드의 상태(객체 생성·삭제, 접속 시 현재 상태 등)를 접속한 클라이언트에게 알리는 함수 모음

// 월드에 배치된 객체(밭, 집, 보트)를 객체마다 S2C_ADD_OBJECT 패킷 하나씩 보낸다. 하나라도 실패하면 false
bool SendWorldObjects(int sessionId);

// SendWorldObjects 이후 호출한다. 밭 상태를 복사한 뒤 월드 잠금 없이 전송한다.
bool SendFieldStates(int sessionId);

// SendFieldStates 이후 호출한다. 기존 접속자와 새 플레이어의 위치·장비를 서로 알리고 준비 완료로 표시한다. 실패하면 false
bool SendPlayerStates(int sessionId);

// Update 스레드에서 호출한다. 각 플레이어에게 마지막으로 보낸 값과 다른 밭만 전송한다.
void SendChangedFieldStates();

// Called by the update thread for initial balances and later changes.
void SendChangedWalletBalances();

// 플레이어의 현재 이동 상태를 초기 동기화가 끝난 다른 접속자에게 보낸다.
void SendPlayerMove(int sessionId);

// 장비 변경이 확인된 경우에만 호출한다. 서버에 저장된 장비를 초기 동기화가 끝난 다른 접속자에게 보낸다.
void SendPlayerEquip(int sessionId);

// 초기 동기화가 끝난 클라이언트에게 객체 하나가 사라졌음을 알린다 (S2C_REMOVE_OBJECT)
void SendRemoveObject(int objectId);
