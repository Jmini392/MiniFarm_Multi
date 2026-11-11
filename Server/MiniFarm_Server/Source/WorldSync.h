#pragma once

// 월드의 상태를 접속한 클라이언트에게 알리는 함수 모음
// 클라이언트와 월드를 맞출 때(객체 생성·삭제, 접속 시 현재 상태 전송 등) 이 파일에 모은다

// 월드에 배치된 객체(밭, 집, 보트)를 객체마다 S2C_ADD_OBJECT 패킷 하나씩 보낸다.
// 하나라도 보내지 못하면 false를 반환한다
bool SendWorldObjects(int sessionId);

// SendWorldObjects 이후 호출한다. 밭 상태를 복사한 뒤 월드 잠금 없이 전송한다.
bool SendFieldStates(int sessionId);
