# 서버 밭 상태 관리

2026-11-07 작업: 서버가 밭별 작물 상태를 보관하고, 객체 ID로 등록·조회하는 기능을 구현했다. 실제 맵의 밭 등록과 패킷 동기화는 후속 작업이다.

- [FieldState.h](MiniFarm_Server/Source/FieldState.h): 밭의 `objectId`와 공용 `CropState`(작물 종류·성장 단계·물·성장 시간)를 보관한다.
- [GameWorld](MiniFarm_Server/Source/GameWorld.h): 객체 ID별로 `FieldState`를 관리한다. `AddField(id)`는 빈 밭을 등록하며 음수·중복 ID를 거부하고 기존 상태를 유지한다. `GetField(id)`는 없는 밭에 `nullptr`를 반환하며 밭을 새로 만들지 않는다.
- 서버 프로젝트에 새 헤더를 등록했다. Debug/x64 재빌드와 별도 실행 검사 9개를 통과했으며, 임시 검사 파일과 빌드 산출물은 정리했다.

후속 연결은 다음 순서로 진행한다. `objectId`는 아이템 종류가 아닌 월드 객체 하나를 식별하는 번호이며, 서버와 클라이언트가 같은 밭에 같은 번호를 사용해야 한다.

1. 서버에서 맵의 밭 객체를 생성할 때 배정한 ID를 `AddField(id)`에 넘겨 밭 상태를 등록한다.
2. `S2C_ADD_OBJECT`로 같은 ID를 전달하고, 클라이언트에서 해당 ID의 `Crop`을 찾을 수 있도록 연결한다.
3. 서버가 `GetField(id)`로 조회한 상태에 농사 판정을 적용하고, `S2C_FIELD_UPDATE`로 같은 ID의 클라이언트 밭을 갱신한다. 수신 상태 적용 시 물·성장 시간이 보존되어야 한다.


