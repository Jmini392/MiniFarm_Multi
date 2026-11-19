# 서버 밭 상태 관리

## 2026-11-18: 개인 수량·공동 잔액 통합 검증

`ksc`의 개인 수량 변경(`82326f9`)과 공동 잔액 변경(`26a5cb0`)을 검증했다. 서버와 클라이언트 모두 Debug/x64 빌드가 통과했다.

| 상황 | 확인 결과 |
| --- | --- |
| A·B가 같은 갈아진 밭에 당근 파종 요청 | 양쪽이 같은 밭 상태를 받고, 성공한 한 사람에게만 개인 수량 갱신 1개가 도착했다. 성공자 당근 씨앗은 1 → 0, 다른 사람은 1 유지 |
| 경작 전 파종, 중복 파종, 다른 갈아진 밭에서 씨앗 부족 | 밭·개인 수량·공동 잔액 유지. 밭·수량·잔액 변경 패킷 없음 |
| 다른 플레이어가 양배추를 파종 | 양배추 씨앗이 줄어든 본인에게만 수량 갱신이 도착했고, 양쪽 밭 상태는 같았다 |
| 새 접속, C/V로 공동 잔액 변경 | 새 서버에서 0을 받았다. A가 10,000G를 추가하면 A·B 모두 10,000을 받았고 후접속자도 10,000으로 시작했다. B가 차감하면 모두 0을 받았다 |
| 두 클라이언트가 잇달아 추가 | 10,000G에서 두 번 추가한 결과 모두 30,000G에 도달했다. 100ms 갱신 사이에 붙어 들어온 변경은 최신 잔액으로 합쳐질 수 있다 |
| 잔액 0에서 추가 차감, 잔액 변화 없이 대기 | 잔액 0 유지. 중복 갱신 패킷 없음 |

기본 포트(9000)에서 저장소의 실제 서버 실행 파일로 TCP 요청과 수신 패킷을 검사했다. 별도의 루프백 포트에서도 현재 빌드한 클라이언트 코드로 OpenGL 렌더링을 실행하며 밭 모델 교체, 개인 인벤토리 적용, C/V 입력 처리와 잔액 수신을 확인했다. 임시 시험 실행 파일은 소스 변경이나 커밋 없이 사용했다. 공동 골드 UI는 11/22 작업이며, 이번 범위의 잔액은 `[Wallet received]` 로그 또는 `GetServerWalletBalance()`에서 확인한다.

### 직접 확인하는 방법

1. 서버와 클라이언트를 **Debug / x64**로 빌드한다. 서버는 `Server/MiniFarm_Server.sln`, 클라이언트는 `Client/MiniFarm.sln`을 연다. 중단점을 사용할 프로그램은 **F5**, 다른 클라이언트는 **Ctrl+F5**로 실행한다. 클라이언트 솔루션 창 하나를 쓴다면 B를 Ctrl+F5로 먼저 띄운 뒤 A를 F5로 실행한다. 두 게임 창에서 각각 **Space**를 눌러 접속한다.
2. 중단점은 아래 표에 나온 줄에 커서를 놓고 **F9**로 건다. 멈추면 **디버그 → 창 → 조사식 → 조사식 1**을 열어 표의 식을 한 줄씩 넣는다. 확인할 때마다 멈춘 Visual Studio에서 **F5**로 재개한다.

| 위치 | F9를 누를 코드 | 조사식 |
| --- | --- | --- |
| 서버 `Source/PacketHandler.cpp` 228줄 | `if (result.status == FieldInteractStatus::PlayerNotReady)` | `sessionId`, `request.objectId`, `result.status`, `result.seedCountBefore`, `result.seedCountAfter`, `inventoryChanged` |
| 클라이언트 `PacketHandler.cpp` 176줄 | `ingame->ApplyInventoryCounts(update.counts);` | `update.counts.carrotSeedCount`, `update.counts.cabbageSeedCount` |
| 서버 `Source/PacketHandler.cpp` 200줄 | `printf("[Wallet debug]..."` | `sessionId`, `before`, `after`, `success` |
| 클라이언트 `Source/Game/IngameScene.cpp` 164줄 | `LOG_D("[Wallet received]..."` | `m_playerId`, `m_serverWalletBalance` |

3. A가 빈 밭에서 **1 → E**, 같은 밭에서 **4 → E**를 누른다. 서버 228줄에서 파종이 `Success`이고 당근 씨앗이 1 → 0인지 확인한다. 재개하면 A의 인벤토리 적용 중단점이 걸린다. A는 하단 당근 씨앗이 x0, B는 x1이며 두 화면에 같은 새싹이 보인다. B가 같은 밭에서 **4 → E**를 누르면 서버는 `NotApplicable`로 판정한다. A가 다른 빈 밭을 **1 → E**로 간 후 **4 → E**를 누르면 `NoSeed`다. 두 실패 모두 `inventoryChanged = false`이고 수량 적용 중단점은 걸리지 않는다.
4. A에서 **C**를 누르고 서버에서 `success = true`, `0 → 10000`을 본다. 재개하면 A·B 콘솔에 각각 `[Wallet received] ... balance=10000`이 찍힌다. 이때 새 클라이언트를 접속하면 첫 잔액도 10000이다. B에서 **V**를 누르면 모두 0을 받는다. B에서 다시 **V**를 누르면 서버에서 `success = false`, `0 → 0`을 볼 수 있고, 새 잔액 수신 로그는 나오지 않는다. 게임 창에 포커스를 두고 Shift 없이 C/V 키를 누른다.

아래 11/16 기록의 수량 UI 상태와 줄 번호는 당시 코드 기준이다.

## 2026-11-16: 두 클라이언트·실패 상황 검증 (3단계, 당시 상태)

앞의 두 단계 커밋 `974dea5`, `e6b24ce`를 기준으로 검증했다. 기능 수정이 필요한 문제는 발견되지 않아 이번 단계에서는 이 문서에 결과와 직접 디버그 절차를 기록했다. 서버·클라이언트 Debug/x64 빌드 통과. 개인 씨앗 수량 UI는 아직 서버 변경을 받지 않으므로, 수량 검증에는 서버 실제 수량을 사용했다.

### 검증 결과

실제 서버 코드에 검증용 상태 조회만 추가한 임시 실행 파일을 사용했다. TCP 클라이언트 두 개의 요청 전후에 밭 18개 전체, 접속자별 개인 아이템 4종, 공동 잔액을 비교했다. 실패 시 이 값들이 그대로인지와 접속 유지 여부를 확인했다. 34개 시나리오의 144개 검사가 통과했다.

| 상황 | 실제 확인 결과 |
| --- | --- |
| A의 밭갈이·당근 파종 | 양쪽에 같은 밭 상태 전달. A의 당근 씨앗만 1 → 0 |
| B의 양배추 파종 | 양쪽에 양배추 상태 전달. B의 양배추 씨앗만 1 → 0 |
| 경작 전 파종·반복 밭갈이 | NotApplicable. 밭·모든 개인 수량·공동 잔액 유지 |
| 같은 사람·다른 사람의 중복 파종 | NotApplicable. 이미 심어진 작물과 실패한 사람의 씨앗 유지 |
| 씨앗 소진 후 다른 갈아진 밭에 파종 | NoSeed. 밭은 Tilled, 씨앗은 0 → 0 |
| 물뿌리개·낫·빈손 | UnsupportedTool. 이번 단계에서는 상태 변경 없음 |
| 음수·없는 ID·상점·보트 ID | UnknownField. 상태 보존 및 접속 유지 |
| 거리 초과·X축 반전 전 좌표·경계 밖 | TooFar. 상태 보존 및 접속 유지 |
| 정확히 거리 2.5인 요청 | 성공. 클라이언트와 같은 3차원 거리 기준 적용 |
| 같은 밭에 동시 파종 7회 | 매회 Success 1개, NotApplicable 1개. 성공한 사람의 씨앗만 차감 |
| 중복 파종 요청 20회 연속 | 모두 거절. 전체 밭·아이템 수량·잔액 유지 |
| 후접속·재접속 | 현재 밭 18개의 종류·단계·물·성장 시간이 초기 수신값과 일치 |
| 크기가 잘못된 밭 요청 | 해당 접속자만 연결 종료. 기존 A·B의 상태와 이후 조작 정상 |

추가로 실제 클라이언트 코드와 OpenGL 컨텍스트를 사용하는 요청자·관찰자·후접속자 실행 파일로 50개 검사가 통과했다. 게임 입력 코드의 E 요청, 실제 서버 판정, 수신 버퍼와 패킷 처리, 밭 상태 적용, 모델 교체까지 확인했다. 사용자 게임 창에서 직접 키를 누르는 수동 검사는 아래 절차로 진행한다. 임시 검증 코드는 저장소에 추가하지 않았으며 검증용 프로세스는 종료했다.

실제 서버 로그 일부:

```text
[Field action] playerId=1, objectId=1000, itemId=6, result=Success, crop=0->1, seed=1->0, distanceSquared=0.000
[Field action] playerId=2, objectId=1000, itemId=6, result=NotApplicable, crop=1->1, seed=1->1, distanceSquared=0.000
[Field action] playerId=1, objectId=1001, itemId=6, result=NoSeed, crop=0->0, seed=0->0, distanceSquared=0.000
[Field action] playerId=1, objectId=1002, itemId=7, result=TooFar, crop=-1->-1, seed=1->1, distanceSquared=120.000
```

### 서버 한 곳에 중단점을 걸어 시작하기

1. 이전 서버·클라이언트를 종료한다. 서버와 클라이언트를 각각 **Debug / x64**로 빌드한다. 서버는 하나만 실행한다.
2. **서버 솔루션 `Server/MiniFarm_Server.sln`의** `Source/PacketHandler.cpp`를 연다. 클라이언트의 같은 이름 파일과 구분한다.
3. `C2S_INTERACT_FIELD` 안의 **`if (result.status == FieldInteractStatus::PlayerNotReady)`** 줄에 F9로 중단점을 건다. 현재 172줄이며 성공·일반적인 실패 모두 판정 직후 이 줄에 도착한다.
4. **서버 Visual Studio에서 `디버그 → 디버깅 시작(F5)`**을 누른다. Ctrl+F5는 중단점으로 멈추지 않는다. 게임 창이 떠 있는 것과 디버거가 붙어 있는 것은 다르다. 디버깅 중에는 Visual Studio에 중지·일시 중지 도구가 표시된다.
5. 별도 클라이언트 Visual Studio에서 Ctrl+F5로 A·B를 실행하고 각각 Space로 접속한다. 게임 창을 클릭하고 밭에 충분히 가까이 가서 잠깐 멈춘 뒤, **숫자 1로 도구 선택 → E로 상호작용**한다. 숫자 1만 누르면 밭 요청은 발생하지 않는다.
6. 서버에 노란 실행 화살표가 나타나면 **디버그 → 창 → 지역**에서 `result`를 펼친다. 보이지 않으면 **디버그 → 창 → 조사식 → 조사식 1**에 `result.status`, `result.before.id`, `result.after.id`, `result.seedCountBefore`, `result.seedCountAfter`, `sessionId`, `request.objectId`를 한 줄씩 입력한다.
7. 각 결과를 확인한 뒤 **F5로 재개**한다. 서버가 중단된 동안에는 다음 요청과 상태 전송도 진행되지 않는다.

### 정상·실패를 같은 중단점에서 순서대로 확인

서버를 새로 시작한 상태에서 진행한다. A·B는 각각 당근 씨앗과 양배추 씨앗을 1개씩 가진다. 아래에서 같은 밭과 다른 빈 밭을 구분한다.

| 순서 | 게임에서 할 일 | result의 기대값 |
| --- | --- | --- |
| 1 | A가 빈 밭에서 4 → E | NotApplicable. Empty → Empty, 씨앗 1 → 1 |
| 2 | A가 그 밭에서 1 → E | Success. Empty → Tilled, 씨앗 -1 → -1 |
| 3 | A가 다시 E | NotApplicable. Tilled → Tilled |
| 4 | A가 같은 밭에서 4 → E | Success. Tilled → Carrot, 당근 씨앗 1 → 0 |
| 5 | B가 이미 심어진 그 밭에서 4 → E | NotApplicable. Carrot → Carrot, B 씨앗 1 → 1 |
| 6 | A가 다른 빈 밭을 1 → E로 갈아 놓은 뒤 4 → E | NoSeed. Tilled → Tilled, A 씨앗 0 → 0 |
| 7 | B가 6번의 갈아진 밭에서 5 → E | Success. Tilled → Cabbage, B 양배추 씨앗 1 → 0 |
| 8 | A가 작물이 있는 밭에서 2 → E 또는 3 → E | UnsupportedTool. 물·작물·수량 유지 |

밭갈이 도구의 seed=-1은 씨앗 장비가 아니라는 표시다. 하단 UI에 씨앗 1이 남아 보여도 6번의 서버 NoSeed 결과가 현재 정상 동작이다. UI 수량 갱신은 11/18 작업이다.

**실패는 새 모델 변경을 만들지 않으므로 클라이언트 Crop.cpp 80줄에서 멈추지 않는 것이 정상이다.** 성공·실패 판정은 위 서버 172줄에서 확인한다. 화면 동기화는 중단점을 해제한 뒤 A·B가 같은 밭을 보는지 확인한다. 동시 요청의 실행 순서를 바꾸지 않으려면 경쟁 테스트 중에는 중단점을 해제하고 서버 로그를 본다. 이번 자동 검증은 두 TCP 요청을 함께 시작하여 검사했다.

### 거리 초과·잘못된 대상 직접 요청 (선택)

정상 게임 입력은 거리 밖의 밭을 미리 걸러내므로 멀리서 E를 누르는 것만으로 서버 TooFar를 재현할 수 없다. 아래는 현재 서버에 별도 테스트 접속 하나를 만들어 초기 위치에서 밭 1000을 요청하는 PowerShell 예시다. 서버 172줄 중단점은 유지한다. 테스트 접속의 초기 위치는 (0, 8, 0)이므로 결과는 TooFar다.

```powershell
$fieldProbeClient = [System.Net.Sockets.TcpClient]::new('127.0.0.1', 9000)
$fieldProbeStream = $fieldProbeClient.GetStream()
# 로그인 → 밭갈이 도구 장착 → 밭 1000 요청
[byte[]]$fieldProbePackets = 2,1, 6,3,3,0,0,0, 6,4,232,3,0,0
$fieldProbeStream.Write($fieldProbePackets, 0, $fieldProbePackets.Length)
```

서버가 멈추면 `result.status=TooFar`를 확인하고 F5로 재개한다. 같은 PowerShell에서 아래를 실행하면 상점 1018을 밭으로 요청하므로 UnknownField가 나와야 한다.

```powershell
[byte[]]$fieldProbeBadTarget = 6,4,250,3,0,0
$fieldProbeStream.Write($fieldProbeBadTarget, 0, $fieldProbeBadTarget.Length)
```

UnknownField 확인 후 F5로 재개하고, 테스트 접속을 아래처럼 닫는다. 게임 플레이어의 연결은 유지된다.

```powershell
$fieldProbeClient.Close()
```

### 클라이언트 중단점 자체가 작동하는지 확인

서버는 Ctrl+F5, **클라이언트 하나는 F5**로 실행한다. 클라이언트 `Crop.cpp` 80줄 대신 **77줄 `m_state = state;`**에 중단점을 걸고 Space로 접속한다. 최초 빈 밭 상태를 받으면서도 멈추므로 모델 변경 조건과 디버거 연결을 분리해 확인할 수 있다. 한 번 멈춘 뒤 77줄 중단점을 해제하고 F5로 재개하면 초기 밭 18개마다 반복해서 멈추는 것을 피할 수 있다. 이후 모델 변경은 80줄에서 확인한다.

이하 내용은 앞 단계의 기록이다. 현재 검증 결과와 실패 확인 방법은 이 3단계 설명을 우선한다.

## 2026-11-16: 기존 요청·밭 상태 전송 연결 확인 (2단계 기록)

- 요청 처리부에서 판정 함수를 호출하는 연결은 앞 단계 커밋 `974dea5`에 포함돼 있다. 이번 단계에서는 기존 전송 경로와 실제 클라이언트의 수신·모델 반영까지 연결을 검증하고, 전송·적용을 구분할 디버그 로그를 보강했다.
- 전체 경로: `Player::HandleInteractInput()` → `Crop::OnInteract()` → `SendInteractFieldRequest()` → 서버 `HandlePacket()` → `GameWorld::InteractField()` → 100ms 갱신의 `SendChangedFieldStates()` → `S2C_FIELD_UPDATE` → 클라이언트 `ApplyFieldUpdate()` → `Crop::ApplyServerState()` → 모델 교체.
- 서버는 요청자도 포함하여 초기 동기화를 마친 접속자에게 변경된 밭만 보낸다. 밭 상태를 복사한 뒤 월드 잠금 없이 송신하며, 성공한 전송만 접속자별 전송 기록에 반영한다. 초기 접속에는 객체 생성 뒤 현재 밭 상태를 보내는 기존 순서를 유지한다.
- 서버 `WorldSync.cpp`에는 전송 결과를 확인할 `sent` 지역 변수를 두고, 실제 변경 전송에 성공하면 Debug 콘솔에 `[Field update sent] receiverId=..., objectId=..., cropId=..., level=...`를 출력한다. 종류나 단계가 바뀐 경우에만 로그를 남겨 성장 시간 갱신 때마다 출력하지 않는다. `receiverId`는 요청자가 아니라 이 패킷을 받는 접속자의 ID다.
- 클라이언트는 수신 상태를 적용하여 모델을 바꾼 뒤 `Field state applied: objectId=..., cropId=..., level=...`를 Debug 콘솔에 출력한다. E 전송 로그는 `awaiting server state`로 표현한다. 전송 성공과 서버 판정·클라이언트 적용 성공을 구분한다.
- 씨앗 부족·중복 파종 등 밭이 바뀌지 않은 요청은 새 밭 갱신을 발생시키지 않는다. 개인 수량 UI 갱신, 물주기·수확, 클라이언트 거부 사유 안내는 이후 일정이다.

### 화면으로 연결 확인

1. 서버와 클라이언트를 **Debug / x64**로 빌드한다. 이전 서버는 종료하고 새 서버를 Ctrl+F5로 실행한다.
2. 클라이언트 A와 B를 각각 Ctrl+F5로 실행하고 Space로 접속한다. 중단점은 모두 해제한다.
3. A에서 가까운 빈 밭을 1 → E로 갈고, 4 → E로 당근을 심는다. 각 입력 뒤 모델이 바뀌는 것을 보고 다음 입력을 한다. A와 B가 같은 밭의 변경을 보아야 한다. 서버 콘솔에는 같은 objectId로 A·B 각각에 대한 전송 로그가 남는다.
4. 클라이언트를 추가로 실행해 접속하면 이미 심어진 당근과 갈아진 밭이 처음부터 같은 상태로 보여야 한다.
5. 하단 씨앗 숫자는 아직 갱신되지 않는다. 서버 수량은 앞 단계의 `result.seedCountBefore/After`로 확인한다.

### 서버에서 변경 패킷 전송 확인

1. 서버 `Source/WorldSync.cpp`의 **`SendChangedFieldStates()` 안**에서 `const bool sent = SendFieldState(sessionId, fields[i]);` 바로 다음 **`if (!sent)`** 줄에 F9로 중단점을 건다. 현재 219줄이다. 다른 함수의 같은 문구와 구분한다.
2. 서버는 F5, 클라이언트 A·B는 Ctrl+F5로 실행한다. A에서 빈 밭에 1 → E를 누른다. 초기 로그인 전송은 이 위치를 지나지 않으므로, 실제 밭 변경을 보낼 때 멈춘다.
3. 서버에 노란 실행 화살표가 나타나면 **디버그 → 창 → 조사식 → 조사식 1**에 아래 값을 한 줄씩 입력한다.

| 조사식 | 밭갈이 시 기대값 |
| --- | --- |
| `sent` | true |
| `sessionId` | 지금 전송한 대상 접속자 ID |
| `fields[i].objectId` | 상호작용한 밭 ID |
| `fields[i].crop.id` | Tilled (0) |
| `fields[i].crop.level` | 0 |

4. F5로 재개하면 다른 접속자에게 보내는 차례에서도 멈출 수 있다. objectId는 같고 sessionId는 다른지 확인한다. 접속자 전송 순서는 고정하지 않는다. 화면을 확인하려면 모든 중단을 F5로 재개한다.
5. 4 → E로 파종했을 때는 `fields[i].crop.id=Carrot (1)`이어야 한다. 실제 패킷 필드를 보고 싶다면 `SendFieldState()`의 `return sent;`에 중단점을 걸어 `update`와 `sent`를 확인할 수 있다. 이 위치는 초기 로그인 전송에서도 멈춘다.

### 받는 클라이언트에서 상태·모델 적용 확인

1. 서버 중단점을 해제하고 서버를 Ctrl+F5로 실행한다. A는 Ctrl+F5, 받는 B는 F5로 실행한다.
2. B의 `Client/MiniFarm/Source/Game/Crop.cpp`에서 **`ApplyServerState()` 안의 `LOG_D("Field state applied: ...`** 줄에 F9로 중단점을 건다. 현재 80줄이며, 상태 대입과 모델 교체가 끝난 위치다.
3. A가 밭갈이 또는 파종하면 B를 디버그하는 Visual Studio가 멈춰야 한다. 지역 창 또는 조사식에서 `m_netId=대상 밭 ID`, `m_state.id=Tilled (0) 또는 Carrot (1)`, `m_state.level=0`, `modelChanged=true`를 확인한다. F5로 재개한 뒤 B 화면의 모델을 확인한다.
4. 패킷 자체를 확인하려면 B의 `IngameScene.cpp`에서 `ApplyFieldUpdate()` 안의 `return crop->ApplyServerState(state);`에 중단점을 걸어 `packet`과 `state`를 확인한다. 이 위치는 모델 적용 전이고, 초기 로그인 때 빈 밭 18개도 처리하므로 필요할 때만 사용한다.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 저장소의 실제 클라이언트 오브젝트와 OpenGL 컨텍스트를 사용하는 임시 실행 파일로, 게임 입력 코드의 E 요청부터 실제 서버 판정·네트워크 수신 버퍼·패킷 처리·밭 상태 적용·모델 교체까지 50개 검사를 통과했다. 요청자와 관찰자의 변경 수신, 후접속자의 현재 모델, 로컬 선행 변경 없음, 서버에서 소진한 씨앗의 재사용 거절을 확인했다. 사용자 게임 창을 직접 조작하는 수동 검사는 위 절차로 진행한다. 임시 검증 코드는 저장소에 추가하지 않았고 검증용 서버와 클라이언트는 종료했다.

이하 1단계와 이전 날짜의 설명은 당시 기록이다. 현재 연결 상태와 디버그 방법은 이 2단계 설명을 우선한다.

## 2026-11-16: 서버 밭갈이·파종 판정 함수 (1단계 기록)

- `GameWorld::InteractField(sessionId, objectId)`가 서버에 저장된 준비 상태, 밭 ID, 위치, 장비, 개인 씨앗 수량을 기준으로 판정한다. 호출부가 `Lock()/Unlock()`으로 전체 호출을 감싸므로 밭 상태 검사·씨앗 차감·파종이 같은 보호 범위에서 처리된다.
- 게임에서 확인할 수 있도록 기존 `C2S_INTERACT_FIELD` 수신부에 함수 호출을 연결했다. 패킷 형식은 유지하며, 밭 변경은 기존 100ms 갱신의 `SendChangedFieldStates()`가 전송한다. 함수 내부에서는 송신하지 않는다.
- 빈 땅 + 밭갈이 도구는 `Empty → Tilled`, 갈아진 밭 + 씨앗은 해당 작물 level 0 생성과 요청자 씨앗 1개 차감을 수행한다. 씨앗 소비는 기존 `CropLogic::Plant()`가 한 번만 처리한다.
- 거리 검사는 클라이언트와 같은 3차원 거리 2.5를 사용한다. 배치 원본의 X만 반전하여 플레이어 이동 패킷 좌표와 비교한다. 1000번 밭의 배치 좌표는 (-8, 4, 14), 화면 좌표는 (8, 4, 14)다. `distanceSquared`는 거리의 제곱이며 허용 범위는 6.25 이하이다.
- `FieldInteractResult`에 결과와 변경 전후 밭 상태·선택한 씨앗 수량을 복사한다. 월드 잠금 해제 뒤에도 안전하게 로그와 디버거에서 확인할 수 있다. 씨앗이 아닌 장비의 수량은 -1로 표시한다.
- 거리 초과·씨앗 부족·중복 파종·잘못된 밭 등 일반적인 행동 실패는 상태를 보존하고 접속을 유지한다. 잘못된 패킷 크기와 준비되지 않은 요청자는 기존 패킷 오류 처리를 따른다.
- `ServerFarmingEnabled=false`에서는 함수도 서버 상태를 변경하지 않는다. 기존 로컬 농사와 상점 분기는 유지한다.
- **화면의 개인 씨앗 수량 갱신은 11/18 작업이다.** 현재는 파종 후 서버 수량만 감소하고 하단 UI 숫자는 유지된다. 물주기·수확과 클라이언트 실패 안내도 이후 일정에서 연결한다. 서버 모드에서 상점 거래는 기존 임시 제한을 유지한다.

### 직접 디버그: E 입력으로 밭갈이·파종 결과 확인

1. 실행 중인 이전 서버·클라이언트를 종료한다. 서버 솔루션 `Server/MiniFarm_Server.sln`과 클라이언트 솔루션 `Client/MiniFarm.sln`을 각각 **Debug / x64**로 빌드한다. `Common/GameConfig.h`의 기본값 true를 사용한다.
2. 서버 `Source/PacketHandler.cpp`에서 `case C2S_INTERACT_FIELD` 안의 **`if (result.status == FieldInteractStatus::PlayerNotReady)`** 줄에 F9로 중단점을 건다. 이 줄은 판정 함수 실행과 월드 잠금 해제가 끝난 다음이다. 이전 수신 확인용 printf 줄은 삭제했으므로 이 위치를 사용한다.
3. 서버는 F5, 별도 Visual Studio 창의 클라이언트는 Ctrl+F5로 실행한다. 게임에서 Space로 접속한다. **서버 F5만 반복하면 밭 요청은 발생하지 않는다. 게임에서 가까운 밭에 E 입력을 해야 한다.**
4. 게임 창을 클릭하고 빈 밭 가까이 이동한다. 상호작용 안내가 보이면 숫자 **1**로 밭갈이 도구를 선택한 뒤 **E**를 한 번 누른다. 서버가 노란 화살표로 멈추면 **디버그 → 창 → 지역**에서 `result`를 펼친다. 지역 창에서 보이지 않으면 **디버그 → 창 → 조사식 → 조사식 1**에 아래 항목을 한 줄씩 입력한다.

| 조사식 | 밭갈이 성공 시 기대값 |
| --- | --- |
| `sessionId` | 요청한 플레이어의 접속 ID |
| `request.objectId` | 상호작용한 밭의 ID |
| `result.status` | Success |
| `result.equippedTool` | Hoe (3) |
| `result.before.id` / `result.after.id` | Empty (-1) → Tilled (0) |
| `result.seedCountBefore` / `result.seedCountAfter` | -1 → -1 (씨앗 장비가 아님) |
| `result.distanceSquared` | 0 이상 6.25 이하 |

5. F5로 재개하면 잠시 뒤 갈아진 밭 모델이 표시된다. 같은 밭에서 숫자 **4**(당근 씨앗)를 선택하고 E를 다시 누른다. 서버가 같은 중단점에서 멈추면 `result.status=Success`, `result.equippedTool=SeedCarrot (6)`, `result.before.id=Tilled (0)`, `result.after.id=Carrot (1)`, `result.seedCountBefore=1`, `result.seedCountAfter=0`인지 확인한다. 숫자키 4와 아이템 ID 6은 서로 다른 값이다.
6. F5로 재개하면 당근 초기 모델이 표시된다. 하단 씨앗 숫자는 아직 1로 보일 수 있다. 서버 차감 확인은 위 결과값으로 한다. 서버가 중단된 동안에는 갱신도 멈추므로 화면 확인 전에 반드시 재개한다.
7. 다시 E를 누르면 `NotApplicable`이며 추가로 소비하지 않는다. 다른 빈 밭을 1+E로 갈고 4+E로 파종하면 이미 씨앗을 사용했으므로 `NoSeed`, `0 → 0`, 밭은 Tilled로 유지되어야 한다. 양배추는 숫자 5이며 초기 수량이 별도로 1개다.
8. 화면 동기화도 확인하려면 중단점을 해제하고 클라이언트를 하나 더 Ctrl+F5로 실행한다. A의 밭갈이·파종이 B에도 반영되는지, 작업 후 접속한 클라이언트에도 현재 상태가 처음부터 보이는지 확인한다.

### 결과 이름과 현재 범위

| 결과 | 의미 |
| --- | --- |
| Success | 밭갈이 또는 파종 성공 |
| NotApplicable | 이미 갈린 밭에 밭갈이, 경작 전 파종, 중복 파종 등 현재 밭 상태에서 불가능 |
| NoSeed | 선택한 씨앗의 서버 수량 부족 |
| TooFar | 서버에 저장된 위치로 계산한 거리가 2.5 초과 |
| UnknownField | 없는 ID, 상점·보트 ID, 배치 정보가 없는 밭 |
| UnsupportedTool | 물뿌리개·낫·빈손 등 이번 단계에서 실행하지 않는 장비 |
| InvalidCropData | 생성할 작물 데이터가 없어 파종을 거절함 |
| ServerFarmingDisabled | 로컬 확인 모드이므로 서버 판정 실행 안 함 |
| PlayerNotReady | 플레이어가 없거나 초기 동기화가 끝나지 않음 |

거리 밖·없는 ID는 정상 게임 입력이 미리 걸러내므로 별도 요청으로 검증했다. 11/14의 PowerShell 수신 확인 요청에는 이동 정보가 없으므로, 지금 그대로 실행하면 초기 위치에서 먼 밭 요청은 TooFar로 거절된다.

검증: 서버 Debug/x64 빌드 통과. 실제 GameWorld 코드를 사용하는 판정 검사 132개, 설정을 false로 둔 별도 빌드 검사 5개, 실제 TCP 서버 검사 10개를 통과했다. 정상·중복·씨앗 부족·거리 경계·X 반전·잘못된 대상, 같은 밭의 동시 파종 및 마지막 씨앗의 중복 소비 방지, 두 접속자의 변경 수신·후접속 초기 상태·행동 실패 후 접속 유지·잘못된 크기의 연결 종료를 확인했다. 임시 테스트 코드는 저장소에 추가하지 않았고 검증용 서버는 종료했다. 실제 게임 창에서 E를 누르는 수동 확인은 위 절차로 진행한다.

이하 날짜별 기록은 당시 구현 상태다. 현재 상태와 디버그 방법은 위 11/16 설명을 우선한다.

## 2026-11-14: 서버 모드 기본 전환·E 입력 연결 (2단계, 당시 기록)

- `Common/GameConfig.h`의 `GameConfig::ServerFarmingEnabled` 기본값을 **true**로 전환했다. 네트워크 밭의 상태·성장은 서버 수신값만 적용한다.
- 가까운 밭에서 E를 한 번 누르면 `Player::HandleInteractInput()` → `Crop::OnInteract()` → `NetworkClient::SendInteractFieldRequest(m_netId)` → 서버 `C2S_INTERACT_FIELD` 처리로 이어진다. 숫자키 장비 처리를 E 입력보다 먼저 수행하므로 같은 프레임의 숫자키+E도 새 도구가 먼저 전송된다.
- E를 길게 눌러도 최초 입력에만 요청한다. 상호작용 거리 밖·대상 없음·상점/보트 UI·T로 켠 UI 모드에서는 밭 요청을 보내지 않는다. 서버 ID나 최초 상태가 준비되지 않은 밭도 요청하지 않으며 로컬 농사로 대신 실행하지 않는다.
- 전송에 실패하면 오류 로그와 기존 연결 종료 처리를 수행한다. 성공 시 `Field request sent: objectId=... (awaiting server handling)` 로그를 남긴다. 전송 성공은 서버의 농사 판정 성공이 아니다.
- **현재 서버는 여전히 수신 확인 단계다.** E로 요청이 도착해도 밭갈이·파종·물주기·수확과 아이템 수량 변경은 아직 실행하지 않는다. 밭이 변하지 않고 씨앗 수량도 그대로인 것이 이 단계의 정상 동작이다. 로컬 물주기 파티클·동작도 먼저 재생하지 않는다.
- 서버 상점 연동 전까지 ON에서는 로컬 구매·판매를 차단한다. 상점에 들어가면 `Trading unavailable`과 `Exit[X]`를 표시하며, B/S를 눌러도 개인 수량과 잔액을 바꾸지 않는다. X로 정상 종료할 수 있다. OFF에서는 기존 상점 거래를 유지한다.
- 이 설정은 농사 전환 설정이다. 시간·공동 재화 UI·보트 탈출까지 서버 연동이 완료되었다는 뜻은 아니다.

### 직접 디버그: 게임에서 E를 눌러 서버 수신 확인

1. 서버와 클라이언트 솔루션을 각각 **Debug / x64**로 다시 빌드한다. 이전 실행 중인 프로그램은 먼저 종료한다.
2. 서버 `Source/PacketHandler.cpp`의 `case C2S_INTERACT_FIELD` 안에서 `printf("[Field request received] ...`가 시작하는 줄에 F9로 중단점을 건다. 이동·로그인 등 다른 중단점은 해제한다.
3. 서버를 **F5**, 클라이언트를 별도 Visual Studio 창에서 **Ctrl+F5**로 실행한다. 클라이언트에서 Space로 접속한다. **서버에서 F5만 반복해도 밭 요청은 발생하지 않는다. 게임에서 아래 E 입력을 해야 해당 중단점에 도착한다.**
4. 클라이언트 게임 창을 클릭하고 숫자 1로 밭갈이 도구를 선택한다. 밭 가까이 이동해 `Press [E] to interact` 안내가 보이면 E를 한 번 누른다. 상점·보트 UI는 닫고, T로 UI 모드를 켰다면 다시 해제한다.
5. 서버가 노란 실행 화살표로 멈추면 **디버그 → 창 → 지역**에서 아래 값을 확인한다. `request`는 펼쳐서 내부 값을 본다.

| 항목 | 기대값 |
| --- | --- |
| `request.header.size` | 6 |
| `request.header.type` | 4 (`C2S_INTERACT_FIELD`) |
| `request.objectId` | 방금 선택한 밭의 서버 ID. 첫 밭은 1000 |
| `sessionId` | 요청한 클라이언트의 접속 ID |
| `playerReady` / `fieldExists` | 둘 다 true |
| `equippedTool` | Hoe (3). 숫자 2로 바꿔 요청하면 WateringCan (4) |

6. 서버를 F5로 재개하면 콘솔에 `[Field request received] ... (receive only)`가 출력된다. E를 떼었다가 다시 눌러야 다음 요청이 발생한다. 밭이나 수량이 변하지 않는 것은 정상이며, 이 단계에서 농사 성공을 확인하는 것은 아니다.
7. 두 클라이언트를 실행했다면 각각 다른 숫자키를 선택해 E를 누른다. `sessionId`와 `equippedTool`이 요청자에 맞게 구분되어야 한다.

### 클라이언트 송신·로컬 상태 확인

서버를 Ctrl+F5, 클라이언트를 F5로 실행한다. `Crop.cpp`의 `const bool sent = network.SendInteractFieldRequest(m_netId);` 다음 줄인 `if (!sent)`에 중단점을 걸고 가까운 밭에서 E를 누른다. 지역 창에서 `sent=true`, `m_netId=대상 밭 ID`, `m_receivedInitialState=true`를 확인한다. 최초 빈 밭이면 `m_state.id=Empty(-1)`이며, 서버에 농사 판정이 붙기 전까지 E 입력으로 이 값이나 개인 수량이 바뀌지 않는다. 양쪽 중단점을 동시에 사용하면 확인 후 양쪽 모두 F5로 재개한다.

### ON/OFF 변경 방법

| 설정 | 농사·성장 | 상점 구매·판매 |
| --- | --- | --- |
| true (현재 기본) | E 요청 전송, 서버에서 받은 밭 상태 표시. 현재 실행 판정은 미구현 | 임시 제한. 수량·잔액 변경 없음 |
| false (로컬 확인용) | 밭 요청 없이 기존 로컬 밭갈이·파종·물주기·성장·수확 | 기존 로컬 거래 유지 |

`Common/GameConfig.h` 한 곳만 수정하고 **서버와 클라이언트를 모두 다시 빌드·재접속**한다. 실행 중 변경이나 서로 다른 모드의 혼용, 기존 플레이 상태 이관은 지원하지 않는다. OFF 확인 후에는 true로 복구하고 양쪽 모두 다시 빌드한다.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 실제 게임의 입력·패킷 처리·OpenGL 코드를 사용하는 임시 검증으로 ON/OFF 동작, 숫자키+E 전송 순서, E 유지 중 중복 방지, UI/거리/초기 상태 검사, 서버 상태 수신 적용, 로컬 계산·거래 차단과 OFF 농사·거래 유지를 확인했다. 실제 서버 연결에서는 다른 접속자에게 장비 변경 순서가 전달되고, 수신 확인 단계의 18개 밭이 그대로 유지되는 것도 확인했다. 임시 검증 코드는 저장소에 추가하지 않았고, 현재 실행 파일은 true 설정으로 빌드되어 있다.

이하 기록은 11/14 당시의 구현 상태와 테스트 방법이다.

## 2026-11-14: 밭 상호작용 요청·서버 수신 (1단계)

- `C2S_INTERACT_FIELD_PACKET`은 6바이트이며 `header`와 대상 밭의 `objectId`만 전송한다. 요청자는 접속 세션으로 구분하고 도구는 서버 `PlayerState`의 장착 정보를 사용한다.
- 클라이언트 `NetworkClient::SendInteractFieldRequest(objectId)`가 기존 `SendAll()`로 전송한다. 음수 ID 또는 연결되지 않은 상태에서는 false를 반환한다. 전송 성공은 서버 농사 성공을 뜻하지 않는다.
- 서버는 패킷 크기, 로그인·초기 동기화가 끝난 플레이어, 존재하는 밭인지 확인한다. 플레이어와 밭은 월드 잠금 안에서 조회하고 로그는 잠금을 해제한 뒤 출력한다.
- 유효한 밭은 `[Field request received] ... (receive only)`로 기록한다. 없는 ID와 상점·보트 ID는 `[Field request ignored]`로 기록하고 접속은 유지한다. 잘못된 패킷 크기나 준비되지 않은 요청자는 기존 패킷 오류 처리에 따라 연결을 종료한다.
- 이번 단계에서는 거리·도구별 농사 가능 여부를 판정하지 않고, 밭과 개인 아이템 수량도 변경하지 않는다. 성공·실패 응답 패킷은 아직 없다.
- **E 입력 자동 전송과 서버 모드 기본값 변경은 다음 작업이다.** 현재 `Common/GameConfig.h`의 `ServerFarmingEnabled`는 여전히 false이며 기존 로컬 농사는 유지된다. 아래 테스트는 E 입력 없이 송수신 경계를 확인하는 절차다.

### 직접 확인: 서버 중단점과 PowerShell 테스트 요청

1. 서버 솔루션 `Server/MiniFarm_Server.sln`을 열고 **Debug / x64**로 빌드한다.
2. `Source/PacketHandler.cpp`의 `case C2S_INTERACT_FIELD` 안에서 `printf("[Field request received] ...`가 시작하는 줄에 F9로 중단점을 건다. 이동·장비 등 다른 중단점은 해제한다.
3. 서버를 F5로 실행한다. 아직 접속자가 없어 중단되지 않는 것이 정상이다.
4. 별도 PowerShell 창에서 아래 코드를 한 번 실행한다. 서버 접속 → 로그인 → 물뿌리개 장착 → 밭 1000번 요청 순서다. 같은 TCP 연결에서 순서대로 처리되므로 별도 대기 시간을 넣지 않는다.

```powershell
$fieldTestClient = [System.Net.Sockets.TcpClient]::new('127.0.0.1', 9000)
$fieldTestStream = $fieldTestClient.GetStream()
[byte[]]$fieldTestPackets = 2,1, 6,3,4,0,0,0, 6,4,232,3,0,0
$fieldTestStream.Write($fieldTestPackets, 0, $fieldTestPackets.Length)
```

5. 서버 Visual Studio에 노란 실행 화살표가 나타나면 **디버그 → 창 → 지역**에서 `request`, `sessionId`, `playerReady`, `fieldExists`, `equippedTool`을 확인한다. 아래 값이면 정상이다.

| 항목 | 기대값 |
| --- | --- |
| `request.header.size` | 6 |
| `request.header.type` | 4 (`C2S_INTERACT_FIELD`) |
| `request.objectId` | 1000 |
| `sessionId` | 방금 접속한 테스트 연결의 ID. 첫 접속이면 1 |
| `playerReady` / `fieldExists` | 둘 다 true |
| `equippedTool` | WateringCan (4) |

6. F5로 재개하면 서버 콘솔에 `[Field request received] playerId=..., objectId=1000, itemId=4 (receive only)`가 출력된다. 밭이 바뀌지 않는 것이 이번 단계의 정상 동작이다.
7. 확인 후 PowerShell에서 `$fieldTestClient.Close()`를 실행한다. 이 연결은 게임 클라이언트가 아니라 수신 확인용 접속이므로 테스트가 끝나면 닫는다.

### 실제 클라이언트 송신 함수까지 직접 확인

서버와 클라이언트를 각각 Debug / x64로 실행한다. 클라이언트도 F5로 디버깅하고 로그인한다. 클라이언트 `Player.cpp`의 `HandleEquipmentInput()` 안에 있는 `SetEquippedTool(selectedItem);` 줄에 중단점을 걸고, 게임에서 현재와 다른 숫자키를 누른다. 클라이언트가 이 줄에서 멈추면 **디버그 → 창 → 직접 실행**에서 다음을 한 번 실행한다.

```cpp
network.SendInteractFieldRequest(1000)
```

반환값 true는 실제 전송 함수가 패킷을 보냈다는 뜻이다. 서버는 위와 같은 중단점에서 멈춰야 하고, `equippedTool`은 방금 선택한 도구여야 한다. 이 위치에서는 장비 변경 패킷이 먼저 전송된 상태다. 서버와 클라이언트 양쪽을 F5로 재개하고, 확인이 끝나면 클라이언트 중단점을 해제한다.

### 모드 전환 계획

- 이후 E 요청 연결과 함께 서버 처리를 기본값으로 전환한다. ON은 요청·서버 판정·상태 수신, OFF는 기존 로컬 농사 확인용으로 유지한다. OFF에서 서버의 농사 상태를 변경하는 요청을 보내지 않는다.
- 설정은 실행 중 전환하는 기능이 아니다. 서버와 클라이언트에 같은 값을 적용하고 모두 다시 빌드·재접속한다. 서로 다른 모드로 실행하거나 실행 중 상태를 이관하는 것은 지원하지 않는다.
- 서버 모드에서 로컬 구매·판매가 개인 수량을 바꾸면 서버 인벤토리와 어긋나므로, 서버 상점 연동 전까지 해당 로컬 거래를 제한하는 처리는 다음 작업에 포함한다.
- 이후 작업: 11/16 밭갈이·파종 및 거리 검사(배치 좌표의 X 반전 고려), 11/18 개인 수량 수신, 11/20 거부 결과, 11/21 물주기·성장, 11/23 수확 통합.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 실제 빌드된 NetworkClient 송신 코드의 TCP 바이트와 서버 수신 함수를 사용하는 33개 검사 통과. 별도 서버 프로세스와 TCP 연결로 요청자 2명, 잘못된 대상의 접속 유지, 늦은 접속자의 초기 밭 상태, 잘못된 크기의 연결 종료 등 12개 검사 통과. 수신 단계에서 밭·개인 수량이 바뀌지 않는 것도 확인했다. 임시 검증 코드는 저장소에 추가하지 않았다.

## 2026-11-13: 후접속 장비 전달 및 통합 확인 (3단계)

- 로그인 응답 → 배치 객체 → 밭 상태 → 기존 플레이어의 위치·장비 순서로 보낸다. `SendPlayerStates(sessionId)`가 기존 `S2C_MOVE_OBJECT`와 `S2C_EQUIP_OBJECT`를 사용하므로 새 패킷 형식은 추가하지 않는다.
- 기존 플레이어 한 명마다 위치 패킷으로 상대를 생성한 뒤 장비 패킷을 적용한다. 기존 플레이어가 다시 움직이거나 장비를 선택하지 않아도 후접속자 화면에 표시된다.
- 기존 접속자에게도 새 플레이어의 위치·장비를 전달한다. 두 명이 동시에 접속해도 먼저 초기화를 마친 플레이어를 기준으로 서로 알린다.
- 객체·밭·플레이어 초기 전송이 모두 끝난 뒤 준비 완료로 표시한다. 호환성을 위해 기존 `m_fieldSyncReady` 및 `GetFieldSyncPlayers()` 이름은 유지한다.
- 초기 플레이어 전송·실시간 이동·장비·퇴장 전송은 `s_playerSyncMutex`로 순서를 맞춘다. 접속 도중 바뀐 최신 장비가 이전 초기값으로 덮이거나, 초기 전송 중 떠난 플레이어가 남는 것을 방지한다.
- 삭제 알림도 준비 완료한 접속자에게만 전송하여 로그인 응답 전에 월드 패킷을 받는 상황을 피한다. 송신 전에는 월드 데이터를 복사하고 잠금을 해제한다.

### 화면으로 후접속 확인

1. 서버와 클라이언트를 Debug / x64로 빌드한다. 서버와 A 클라이언트를 Ctrl+F5로 실행한다.
2. A에서 Space로 접속하고, 다른 캐릭터와 겹치지 않게 몇 걸음 움직인 뒤 숫자 2로 물뿌리개를 선택한다.
3. 그 뒤 B 클라이언트를 실행하여 Space로 접속한다. A는 장비를 다시 누르지 않는다. B 화면에 A가 기존 위치에서 물뿌리개를 들고 있어야 한다.
4. A 화면에도 B가 나타나는지 확인한다. 이후 A와 B에서 장비를 바꾸면 서로의 화면에 반영되어야 한다.
5. A가 씨앗을 선택한 상태에서 B를 종료·재접속하면 B 화면에서 A는 손 도구 없이 보여야 한다. 도구 선택 상태에서 재접속하면 해당 도구가 보여야 한다.

### 서버에서 초기 장비 전송 디버그

1. `Server/MiniFarm_Server/Source/WorldSync.cpp`의 `SendPlayerStates()` 안에서 `const bool sent = SendToClient(sessionId, (const char*)&state.equip);` 바로 다음 줄인 `if (!sent)`에 F9로 중단점을 건다. `SendPlayerEquip()`의 실시간 전송 줄과 구분한다.
2. 서버는 F5로 실행하고 A는 Ctrl+F5로 실행한다. A가 먼저 접속하여 숫자 2로 물뿌리개를 선택한다. 기존 접속자가 없는 첫 로그인에서는 이 반복문이 실행되지 않으므로 중단되지 않는다.
3. B를 Ctrl+F5로 실행하여 Space로 접속한다. 이번에는 기존 A의 장비를 보내면서 서버가 중단되어야 한다.
4. 디버그 → 창 → 조사식 → 조사식 1에서 `sessionId`, `state.move`, `state.equip`, `sent`를 확인한다. `sessionId`는 후접속한 B의 ID, 두 패킷의 `objectId`는 기존 A의 ID, `state.equip.itemId`는 4, `header.size`는 10, `header.type`은 12, `sent`는 true여야 한다.
5. F5로 재개하고 B 화면에서 A의 물뿌리개를 확인한다. 서버를 중단한 동안에는 초기 전송이 끝나지 않으므로 반드시 재개한다.
6. 클라이언트 적용값도 확인하려면 서버 중단점을 해제하고, 받는 B를 F5로 실행하여 `IngameScene::ApplyEquipObject()`의 `otherplayer.SetEquippedTool(item);`에 중단점을 건다. B 접속만으로 멈추는지 확인하고 F10으로 적용한 뒤 F5로 재개한다.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 실제 TCP 서버로 입력 없이 유지된 기존 플레이어의 위치·장비, 빈손, 3번째 접속, 새 플레이어 알림, 실시간 변경, 미로그인 세션 제외를 확인했다. 빠른 장비 변경 중 접속 12회, 동시 접속, 초기 접속 중 퇴장 6회에서 최신 장비 유지와 퇴장 객체 정리를 확인했다. 실제 클라이언트 코드로도 후접속 초기 수신 → 상대 생성 → 장비 적용 → OpenGL 도구 표시와 기존 수신 검증을 포함한 61개 검사를 통과했다. 임시 서버는 종료했고 테스트 코드는 저장소에 추가하지 않았다.

## 2026-11-13: 받은 장비를 상대 캐릭터에 적용 (2단계)

- 클라이언트 `PacketHandler.cpp`가 `S2C_EQUIP_OBJECT_PACKET`의 크기를 검사하고 `IngameScene::ApplyEquipObject()`로 넘긴다. 플레이어 ID와 허용 장비 종류를 검사한다.
- 상대 객체가 있으면 `SetEquippedTool()`로 적용한다. 기존 `Player::Draw()`를 재사용하여 밭갈이 도구·물뿌리개·낫을 표시하고, 씨앗 또는 Empty 선택 시 손 도구를 숨긴다.
- 상대가 첫 이동 패킷으로 생성되기 전에 장비를 받으면 `m_pendingEquipment`에 플레이어별 마지막 장비를 보관한다. 생성 시 적용하고 보관값을 지운다. 퇴장과 씬 초기화 시에도 보관값을 지운다.
- 내 플레이어의 장비 에코는 무시한다. 상대에게 내 입력·이동·농사를 실행하는 `Player::Update()`를 호출하지 않으며, 기존 `OtherPlayer::Update()`를 유지한다.
- 이후 3단계에서 후접속 시 현재 장비 전송까지 연결했다. 아래 절차는 접속 후 장비 변경 적용을 확인하는 방법이며, 후접속 검사는 위 3단계 절차를 사용한다.

### 화면으로 확인

1. 서버와 클라이언트를 Debug / x64로 빌드한다. 서버 솔루션은 Ctrl+F5로 실행한다.
2. 클라이언트 솔루션에서 Ctrl+F5로 A를 실행하고, F5로 B를 실행한다. 두 창에서 각각 Space로 접속한다.
3. A를 몇 걸음 움직여 캐릭터가 겹치지 않게 한 뒤, A에서 숫자 1 → 2 → 3을 누른다. B 화면에서 A가 든 도구가 밭갈이 도구 → 물뿌리개 → 낫으로 바뀌어야 한다.
4. A에서 4 또는 5를 선택하면 B 화면의 A 손 도구가 사라져야 한다. B에서도 장비를 바꾸면 A 화면에 반영되어야 한다.

### 받는 클라이언트에서 디버그

1. `Client/MiniFarm/Source/Game/IngameScene.cpp`의 `ApplyEquipObject()` 안에 있는 `otherplayer.SetEquippedTool(item);` 줄에 F9로 중단점을 건다. `ApplyMoveObject()` 안의 생성 시 적용 줄과 구분한다.
2. A는 Ctrl+F5, B는 F5로 실행하고 둘 다 접속한다. A에서 2를 누른다. 이미 물뿌리개라면 1을 누른 뒤 2를 누른다.
3. B를 디버그하는 Visual Studio가 노란 화살표로 멈추면, 디버그 → 창 → 조사식 → 조사식 1에 `packet`, `item`, `otherplayer`를 각각 입력한다.
4. `packet.objectId`는 장비를 바꾼 A의 ID, `packet.itemId`는 4, `item`은 WateringCan이어야 한다.
5. F10을 한 번 눌러 장비 적용 줄을 실행한다. `otherplayer`의 Player 기본 클래스 안에 있는 `m_equippedTool`이 WateringCan(4)로 바뀌는지 확인한다. 조사식에 `otherplayer.m_equippedTool`을 직접 입력해도 된다.
6. F5로 재개하면 B 화면에 A의 물뿌리개가 표시된다. 중단 중에는 B 화면 갱신이 멈추므로 재개 후 확인한다.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 실제 서버 → NetworkClient 수신 버퍼 → HandlePacket → IngameScene → OtherPlayer 경로를 사용하는 임시 클라이언트로 검증했다. 장비가 이동보다 먼저 도착한 경우 마지막 값을 적용했고, 세 도구의 실제 OpenGL 렌더링이 서로 다르며 씨앗·Empty는 동일한 빈손 표시임을 확인했다. 내 장비·카메라 유지, 이동 후 장비 유지, 실제 퇴장, 보관값 소비·삭제·씬 초기화, 여러 상대의 생성 순서 차이, 잘못된 ID·장비·패킷 크기 및 내 장비 에코도 확인했다. 총 61개 검증을 통과했고 임시 테스트 서버는 종료했다. 테스트 코드는 저장소에 추가하지 않았다.

## 2026-11-13: 장비 변경 서버 전송 (1단계)

- `S2C_EQUIP_OBJECT_PACKET`은 10바이트이며, `objectId`(플레이어 세션 ID)와 `itemId`를 보낸다.
- 서버 `C2S_EQUIP` 처리에서 기존 장비 검사·저장에 성공하고 값이 실제로 바뀌면 `SendPlayerEquip(sessionId)`를 호출한다. 같은 장비의 중복 요청은 전송하지 않는다.
- `WorldSync.cpp`에서 서버에 저장된 장비와 초기 객체·밭 동기화를 마친 수신자 목록을 복사한다. 월드 잠금을 해제한 뒤 본인을 제외한 수신자에게 기존 `SendToClient()` 경로로 전송한다.
- 1단계에서 서버 전송, 2단계에서 클라이언트 수신·상대 손 도구 적용, 3단계에서 후접속 시 현재 장비 전송을 연결했다.
- `Common/GameConfig.h`의 `ServerFarmingEnabled = false`는 유지한다.

### Visual Studio에서 직접 확인

1. 서버 솔루션 `Server/MiniFarm_Server.sln`을 **Debug / x64**로 연다.
2. `Source/WorldSync.cpp`의 `SendPlayerEquip()`에서 `const bool sent = SendToClient(receiverId, (const char*)&update);` 바로 다음 줄인 `if (!sent)`에 **F9**로 중단점을 건다. 이 함수 안의 줄인지 확인한다.
3. 서버를 **F5**로 실행한다. 별도 Visual Studio에서 클라이언트 솔루션 `Client/MiniFarm.sln`을 Debug / x64로 빌드하고 **Ctrl+F5**를 두 번 사용해 클라이언트 두 개를 실행한다. 각각 Space로 접속한다.
4. 한쪽 클라이언트에서 숫자 **2**를 누른다. 기본 장비가 비어 있는 상태에서 시작하거나, 이미 물뿌리개를 선택했다면 1을 누른 뒤 2를 누른다.
5. 서버가 노란 화살표로 멈추면 **디버그 → 창 → 지역**에서 `update`를 펼친다. `header.size=10`, `header.type=12`, `itemId=4`(물뿌리개)를 확인한다. `update.objectId`와 `sessionId`는 장비를 바꾼 사람의 ID, `receiverId`는 다른 사람의 ID이며 `sent=true`여야 한다.
6. 지역 창에서 찾기 어렵다면 **디버그 → 창 → 조사식 → 조사식 1**에 `update`, `sessionId`, `receiverId`, `sent`를 각각 한 줄씩 적는다. 명령을 실행하는 직접 실행 창은 필요 없다.
7. **F5**로 재개하고 다른 클라이언트에서도 1 또는 3을 눌러 확인한다. 이번에는 송신자와 수신자 ID가 바뀌어야 한다. 중단점을 해제하거나 다시 F5로 재개해야 게임이 계속된다.

| 입력 키 | 장비 | itemId |
| --- | --- | ---: |
| 1 | 밭갈이 도구 | 3 |
| 2 | 물뿌리개 | 4 |
| 3 | 낫 | 5 |
| 4 | 당근 씨앗 | 6 |
| 5 | 양배추 씨앗 | 7 |

`sent=true`는 서버가 패킷 전체를 소켓에 보냈다는 뜻이다. 상대 화면 적용까지 확인하려면 위 2단계 절차를 사용한다. 수신값만 확인하려면 받는 클라이언트의 `PacketHandler.cpp`에서 `return ingame->ApplyEquipObject(equip);`에 중단점을 걸고 `equip`을 확인한다. 이때 서버의 중단점은 해제해 둔다.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 실제 서버에 TCP 테스트 클라이언트를 연결해 모든 허용 장비 값, 본인·미로그인 세션 제외, 양방향·다중 수신자 전달, 중복 요청 생략, 로그인 전 요청·잘못된 장비·잘못된 크기 거부를 확인했다. 이동 전달도 함께 확인했다. 임시 테스트 서버는 종료했고 테스트 코드는 저장소에 추가하지 않았다.


## 현재 기본 실행: 기존 로컬 농사 유지

- 공통 설정은 `Common/GameConfig.h`의 `GameConfig::ServerFarmingEnabled` 한 곳에서 관리한다. 기본값은 **false**이다. 서버 농사 요청과 판정이 준비되면 true로 전환하고 서버와 클라이언트를 모두 다시 빌드한다.
- **false:** 접속 시 밭 초기 상태는 적용한다. 이후 서버의 밭 변경 전송은 중지하며, 클라이언트도 두 번째 이후의 유효한 밭 상태 패킷으로 로컬 농사를 덮어쓰지 않는다. 기존 밭갈이·파종·물주기·성장·수확과 수량 변경이 동작한다.
- **true:** 서버의 변경된 밭 상태를 계속 전달하고 클라이언트의 로컬 농사·성장 계산을 막는다. 서버 농사 요청·판정이 아직 미연결이므로 현재 이 설정은 동기화 검사 때만 사용한다.
- 네트워크 객체 ID와 서버 농사 사용 여부를 구분했다. 객체 생성 및 이후 장비 동기화 작업은 이 설정과 별개로 이어갈 수 있다.
- 기본 실행에서 농사는 아직 클라이언트별 로컬 처리이며 다른 플레이어와 농사 결과나 수량이 공유되는 단계는 아니다.

### 기본 실행 직접 확인

1. `Common/GameConfig.h`에서 `ServerFarmingEnabled = false`를 확인하고 서버·클라이언트를 Debug/x64로 다시 빌드한다.
2. 이전 성장·패킷 중단점은 끄고 서버와 클라이언트를 실행한 뒤 Space로 접속한다.
3. 평소처럼 밭갈이 도구 선택 후 E로 밭을 갈고, 씨앗 심기 → 물주기 → 성장 → 수확을 확인한다. 파종 시 씨앗이 줄고 수확 시 수확물이 늘어야 한다.
4. 이때 서버 조사식에서 밭을 변경해도 기본 실행 중인 클라이언트의 로컬 농사가 덮어써지지 않는 것이 정상이다. 서버 성장 동기화 자체를 확인하려면 아래의 true 설정 검사 절차를 사용한다.

최종 검증: false 기본 설정으로 서버·클라이언트 Debug/x64를 빌드하고 실제 서버 접속 후 밭갈이·파종·물주기·두 단계 성장·수확·수량 변경·수신 상태 덮어쓰기 방지 등 19개 검사를 통과했다. true 설정에서도 기존 클라이언트 검사 36개와 다중 접속 성장 동기화 검사가 통과했다. 최종 설정과 실행 파일은 false로 다시 빌드한 상태다. 검사 코드는 저장소에 추가하지 않았다.

## 2026-11-11: 플레이 중 밭 상태 변경 전파 (2단계)

- 기존 Update 스레드가 100ms마다 서버 작물을 계산한 뒤 `SendChangedFieldStates()`를 호출한다.
- 클라이언트별 마지막 전송 상태와 비교해 바뀐 밭만 보낸다. 작물 종류·단계뿐 아니라 물과 성장 시간의 변화도 포함한다. 변화가 없으면 패킷을 보내지 않는다.
- `PlayerState::m_fieldSyncReady`는 현재 객체·밭·플레이어 초기 전송을 모두 마친 뒤 `SendPlayerStates()`에서 켜진다. 밭 전송 순서는 별도 잠금으로 관리하고, 플레이어 초기 상태·이동·장비·퇴장은 같은 플레이어 전송 잠금으로 순서를 맞춘다. 송신 중 월드 잠금은 잡지 않는다.
- 초기 전송 중에 서버 상태가 바뀌어도 다음 갱신에서 해당 클라이언트가 받은 값과 비교해 보완한다. 퇴장한 접속자의 전송 기록도 정리한다.
- 서버 농사 설정이 true인 경우 네트워크 밭(`m_netId >= 0`)은 `ApplyServerState()`로만 상태를 적용한다. 자체 성장, 밭갈이·파종·물주기·수확과 그에 따른 인벤토리 변경을 실행하지 않는다. 설정이 false이면 기존 로컬 농사 처리를 유지한다.
- E 입력의 서버 요청 전송과 농사 판정은 이후 작업이다. 현재 true 설정에서는 E 입력으로 밭이나 개인 수량이 바뀌지 않으며, 기본 false 설정에서는 기존 로컬 농사가 실행된다.

### 직접 확인: 실제 서버 상태 변경과 성장

1. `Common/GameConfig.h`의 `ServerFarmingEnabled`를 **true**로 바꾸고 서버와 클라이언트를 **Debug / x64**로 다시 빌드한다. 이전 `WorldSync.cpp` 패킷 수정 중단점은 꺼둔다.
2. 서버를 F5로 실행한다. 클라이언트를 하나 또는 두 개 별도로 실행하고 각각 Space로 접속한다.
3. 서버 `Source/GameWorld.cpp`의 `GameWorld::Update()`에서 `const CropData* data = m_cropTable.GetCrop(crop.id);` 줄에 중단점을 놓는다. 조건은 `pair.second.objectId == 1000`이다. 밭을 등록하는 코드가 아니라 성장 반복문 안의 줄이다.
4. 노란 실행 화살표가 이 줄에서 멈추면 조사식에 `crop`을 입력하고 펼친다. `crop.id = EItemID::Carrot`, `level = 0`, `water = 25`, `growTime = 0`, `requiredGrowTime = 3`으로 값을 바꾼다. 이번에는 패킷 복사본이 아닌 서버 밭의 원본 상태를 수정한다.
5. 이 중단점을 끄고 F5로 재개한다. 밭 1000번(클라이언트 좌표 `(8, 4, 14)`)에 당근이 생기고 성장 진행 바가 움직인 뒤 중간 단계와 성숙 모델로 바뀌어야 한다. 두 클라이언트가 있다면 같은 진행과 단계가 보여야 한다. 다른 밭은 빈 땅으로 유지된다. 중단점에서 멈춘 시간이 서버 경과 시간에 포함되므로 첫 단계는 빠르게 바뀔 수 있다.
6. 성장 중 서버를 다시 중단하고 클라이언트를 잠시 두었을 때 진행 바가 자체적으로 움직이지 않는지 확인한다. E 입력으로도 밭 상태와 인벤토리 수량이 바뀌지 않아야 한다.
7. 송신값은 `WorldSync.cpp`의 `SendFieldState()` 안 `update`, 수신값은 클라이언트 `PacketHandler.cpp`의 `update`, 적용값은 `Crop::ApplyServerState()`의 `m_state`로 확인한다. 이때 값을 수정하지 않고 비교한다.
8. 검사가 끝나면 설정을 **false**로 되돌리고 서버와 클라이언트를 다시 빌드해 기존 농사가 가능한 기본 실행으로 복귀한다.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 실제 서버와 클라이언트 코드로 수신·모델 교체·로컬 농사 및 성장 차단 등 36개 검사가 통과했다. 두 TCP 접속자에게 성장 진행·두 단계 변경·빈 밭 복귀가 동일하게 전파되고, 도중 접속·다른 접속자 종료 후 갱신이 정상임을 확인했다. 로그인 전에는 갱신을 받지 않고, 변화가 없으면 반복 패킷이 발생하지 않는 것도 확인했다. 임시 검사 코드는 저장소에 추가하지 않았다.

## 2026-11-11: 접속 시 밭 상태 적용 (1단계)

- 서버 로그인 응답 이후 `S2C_ADD_OBJECT`로 객체 21개를 생성하고, `S2C_FIELD_UPDATE`로 밭 18개의 현재 상태를 보낸다.
- `WorldSync::SendFieldStates()`는 월드 잠금 안에서 밭 상태를 복사하고, 잠금을 해제한 뒤 전송한다.
- 패킷은 26바이트이며 `objectId`, `cropId`, `level`, `water`, `growTime`, `requiredGrowTime`을 전달한다.
- 클라이언트 `IngameScene::ApplyFieldUpdate()`는 같은 `m_netId`의 `Crop`을 찾는다. `Crop::ApplyServerState()`는 수신값을 검증하고 물·성장 시간을 보존해 적용한다. 작물 종류나 단계가 바뀌면 모델을 교체한다.
- 1단계는 접속 시 상태 적용까지 구현한 작업이다.

### 직접 확인

1. 서버와 클라이언트를 모두 **Debug / x64**로 빌드한다.
2. 서버 `Source/WorldSync.cpp`의 `SendFieldState()`에서 `return SendToClient(sessionId, (const char*)&update);` 줄에 F9로 중단점을 놓는다. 조건을 `update.objectId == 1000`으로 지정한다.
3. 서버를 F5로 실행하고, 클라이언트를 별도로 실행한 뒤 Space로 접속한다. 서버에서 노란 실행 화살표가 멈추면 **디버그 → 창 → 조사식 → 조사식 1**을 연다.
4. 조사식에 `update`를 추가하고 펼친다. 기본값은 `objectId=1000`, `cropId=-1`, `level=0`, `water=0`, `growTime=0`, `requiredGrowTime=3`이다.
5. 눈으로 구분하려면 이 시점에 **전송할 패킷의 값만** `cropId=1`, `level=1`, `water=7.5`, `growTime=1.5`, `requiredGrowTime=3`으로 바꾸고 F5로 재개한다. 서버 월드의 원본 상태를 바꾸는 작업은 아니다.
6. 밭 1000번은 클라이언트 좌표 `(8, 4, 14)`에 있다. 당근 중간 단계 모델과 물·성장 진행률 각각 50%가 표시되고, 다른 밭은 빈 땅이어야 한다. 이 물 수치는 다음 성장에 필요한 물보다 적으므로 기다려도 로컬 성장으로 단계가 바뀌지 않는다.
7. 클라이언트도 디버깅하려면 `PacketHandler.cpp`의 `return ingame->ApplyFieldUpdate(update);`에 중단점을 걸어 같은 값을 받는지 확인한다. `Crop.cpp`의 `m_state = state;`를 F10으로 실행한 뒤 `m_state`를 펼치면 적용된 값도 확인할 수 있다.

검증: 서버·클라이언트 Debug/x64 빌드 통과. 실제 서버의 동시 접속 두 개에서 초기 인벤토리와 객체 생성 후 밭 상태 전송 순서를 확인했다. 실제 클라이언트 수신 경로, 모델 교체, 진행률 보존, 잘못된 상태 거부 등 임시 검사 27개가 통과했다. 검사 코드는 저장소에 추가하지 않았다.

## 2026-11-07 작업 기록

2026-11-07 작업: 서버가 밭별 작물 상태를 보관하고, 객체 ID로 등록·조회하는 기능을 구현했다. 실제 맵의 밭 등록과 패킷 동기화는 후속 작업이다.

- `FieldState`: 밭의 `objectId`와 공용 `CropState`(작물 종류·성장 단계·물·성장 시간)를 보관한다. 처음에는 `FieldState.h`에 있었고, 2026-11-17에 [GameWorld.h](MiniFarm_Server/Source/GameWorld.h)로 옮겼다.
- [GameWorld](MiniFarm_Server/Source/GameWorld.h): 객체 ID별로 `FieldState`를 관리한다. `AddField(id)`는 빈 밭을 등록하며 음수·중복 ID를 거부하고 기존 상태를 유지한다. `GetField(id)`는 없는 밭에 `nullptr`를 반환하며 밭을 새로 만들지 않는다.
- 서버 프로젝트에 새 헤더를 등록했다. Debug/x64 재빌드와 별도 실행 검사 9개를 통과했으며, 임시 검사 파일과 빌드 산출물은 정리했다.

후속 연결은 다음 순서로 진행한다. `objectId`는 아이템 종류가 아닌 월드 객체 하나를 식별하는 번호이며, 서버와 클라이언트가 같은 밭에 같은 번호를 사용해야 한다.

1. 서버에서 맵의 밭 객체를 생성할 때 배정한 ID를 `AddField(id)`에 넘겨 밭 상태를 등록한다.
2. `S2C_ADD_OBJECT`로 같은 ID를 전달하고, 클라이언트에서 해당 ID의 `Crop`을 찾을 수 있도록 연결한다.
3. 서버가 `GetField(id)`로 조회한 상태에 농사 판정을 적용하고, `S2C_FIELD_UPDATE`로 같은 ID의 클라이언트 밭을 갱신한다. 수신 상태 적용 시 물·성장 시간이 보존되어야 한다.


