#pragma once
#include "Common.h"
#include <unordered_map>
#include "FieldState.h"
#include "PlayerState.h"
#include "../../../Common/CropTable.h"
#include "../../../Common/InventoryCore.h"
#include "../../../Common/SharedWallet.h"

// 서버가 가지고 있는 게임 월드
// 공용 로직(Common)이 변경할 상태를 이곳에서 가진다
// 여러 스레드가 함께 쓰므로, 월드를 읽거나 바꾸는 동안에는 Lock()/Unlock()으로 감싸야 한다
class GameWorld {
public:
	GameWorld() { InitializeCriticalSection(&m_cs); }
	~GameWorld() { DeleteCriticalSection(&m_cs); }
	GameWorld(const GameWorld&) = delete;
	GameWorld& operator=(const GameWorld&) = delete;

	// 월드를 쓰는 동안 다른 스레드가 접근하지 못하게 막는다
	void Lock() { EnterCriticalSection(&m_cs); }
	void Unlock() { LeaveCriticalSection(&m_cs); }

	// 작물 데이터(crops.csv)를 읽어 월드를 준비한다. 실패하면 false
	bool Init(const char* cropCsvPath);

	// 접속한 플레이어의 개인 인벤토리를 만들고, 나가면 지운다
	void AddPlayer(int sessionId);
	void RemovePlayer(int sessionId);

	// 플레이어 조회
	PlayerState* GetPlayer(int sessionId);

	// 해당 플레이어의 인벤토리, 없으면 nullptr
	InventoryCore* GetInventory(int sessionId);

	// 빈 밭을 등록한다. 음수 ID나 중복 ID는 거부하며 기존 상태를 유지한다.
	bool AddField(int objectId);
	// 해당 밭의 상태, 없으면 nullptr. 조회만으로 밭을 만들지 않는다.
	FieldState* GetField(int objectId);

	// 월드에 하나뿐인 공동 지갑
	SharedWallet& GetWallet() { return m_wallet; }
	const CropTable& GetCropTable() const { return m_cropTable; }
	int GetPlayerCount() const { return (int)m_players.size(); }

	// 시간이 지난 만큼 월드를 갱신한다 (밭의 작물 성장). dtMs는 밀리초. 호출하는 쪽이 Lock을 잡아야 한다
	void Update(int dtMs);

	// TODO: 배치 데이터 로딩 후 밭 등록, 상점·보트 같은 월드 오브젝트 관리

private:
	CRITICAL_SECTION m_cs; // 월드 보호용 임계영역
	CropTable m_cropTable; // 작물 설정
	SharedWallet m_wallet; // 모든 플레이어가 함께 쓰는 돈
	std::unordered_map<int, PlayerState> m_players; // 세션 ID별 플레이어 상태
	std::unordered_map<int, FieldState> m_fields; // 월드 객체 ID별 밭 상태
};
