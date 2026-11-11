# 서버 밭 상태 관리

## 2026-11-11: 접속 시 밭 상태 적용

- 서버 로그인 응답 이후 `S2C_ADD_OBJECT`로 객체 21개를 생성하고, `S2C_FIELD_UPDATE`로 밭 18개의 현재 상태를 보낸다.
- `WorldSync::SendFieldStates()`는 월드 잠금 안에서 밭 상태를 복사하고, 잠금을 해제한 뒤 전송한다.
- 패킷은 26바이트이며 `objectId`, `cropId`, `level`, `water`, `growTime`, `requiredGrowTime`을 전달한다.
- 클라이언트 `IngameScene::ApplyFieldUpdate()`는 같은 `m_netId`의 `Crop`을 찾는다. `Crop::ApplyServerState()`는 수신값을 검증하고 물·성장 시간을 보존해 적용한다. 작물 종류나 단계가 바뀌면 모델을 교체한다.
- 이번 단계는 접속 시 상태 적용까지다. 플레이 중 변경 전파와 클라이언트 자체 농사·성장 처리 정리는 다음 작업이다.

### 직접 확인

1. 서버와 클라이언트를 모두 **Debug / x64**로 빌드한다.
2. 서버 `Source/WorldSync.cpp`의 `SendFieldStates()`에서 `if (!SendToClient(sessionId, (const char*)&update))` 줄에 F9로 중단점을 놓는다. 조건을 `update.objectId == 1000`으로 지정한다.
3. 서버를 F5로 실행하고, 클라이언트를 별도로 실행한 뒤 Space로 접속한다. 서버에서 노란 실행 화살표가 멈추면 **디버그 → 창 → 조사식 → 조사식 1**을 연다.
4. 조사식에 `update`를 추가하고 펼친다. 기본값은 `objectId=1000`, `cropId=-1`, `level=0`, `water=0`, `growTime=0`, `requiredGrowTime=3`이다.
5. 눈으로 구분하려면 이 시점에 **전송할 패킷의 값만** `cropId=1`, `level=1`, `water=7.5`, `growTime=1.5`, `requiredGrowTime=3`으로 바꾸고 F5로 재개한다. 서버 월드의 원본 상태를 바꾸는 작업은 아니다.
6. 밭 1000번은 클라이언트 좌표 `(8, 4, 14)`에 있다. 당근 중간 단계 모델과 물·성장 진행률 각각 50%가 표시되고, 다른 밭은 빈 땅이어야 한다. 이 물 수치는 다음 성장에 필요한 물보다 적으므로 기다려도 로컬 성장으로 단계가 바뀌지 않는다.
7. 클라이언트도 디버깅하려면 `PacketHandler.cpp`의 `return ingame->ApplyFieldUpdate(update);`에 중단점을 걸어 같은 값을 받는지 확인한다. `Crop.cpp`의 `m_state = state;`를 F10으로 실행한 뒤 `m_state`를 펼치면 적용된 값도 확인할 수 있다.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 실제 서버의 동시 접속 두 개에서 초기 인벤토리와 객체 생성 후 밭 상태 전송 순서를 확인했다. 실제 클라이언트 수신 경로, 모델 교체, 진행률 보존, 잘못된 상태 거부 등 임시 검사 27개가 통과했다. 검사 코드는 저장소에 추가하지 않았다.

## 2026-11-07 작업 기록

2026-11-07 작업: 서버가 밭별 작물 상태를 보관하고, 객체 ID로 등록·조회하는 기능을 구현했다. 실제 맵의 밭 등록과 패킷 동기화는 후속 작업이다.

- [FieldState.h](MiniFarm_Server/Source/FieldState.h): 밭의 `objectId`와 공용 `CropState`(작물 종류·성장 단계·물·성장 시간)를 보관한다.
- [GameWorld](MiniFarm_Server/Source/GameWorld.h): 객체 ID별로 `FieldState`를 관리한다. `AddField(id)`는 빈 밭을 등록하며 음수·중복 ID를 거부하고 기존 상태를 유지한다. `GetField(id)`는 없는 밭에 `nullptr`를 반환하며 밭을 새로 만들지 않는다.
- 서버 프로젝트에 새 헤더를 등록했다. Debug/x64 재빌드와 별도 실행 검사 9개를 통과했으며, 임시 검사 파일과 빌드 산출물은 정리했다.

후속 연결은 다음 순서로 진행한다. `objectId`는 아이템 종류가 아닌 월드 객체 하나를 식별하는 번호이며, 서버와 클라이언트가 같은 밭에 같은 번호를 사용해야 한다.

1. 서버에서 맵의 밭 객체를 생성할 때 배정한 ID를 `AddField(id)`에 넘겨 밭 상태를 등록한다.
2. `S2C_ADD_OBJECT`로 같은 ID를 전달하고, 클라이언트에서 해당 ID의 `Crop`을 찾을 수 있도록 연결한다.
3. 서버가 `GetField(id)`로 조회한 상태에 농사 판정을 적용하고, `S2C_FIELD_UPDATE`로 같은 ID의 클라이언트 밭을 갱신한다. 수신 상태 적용 시 물·성장 시간이 보존되어야 한다.


