#pragma once
#include "PlayerState.h"
#include "TimeSystem.h"
#include "Crow.h"
#include "../../../Common/CropState.h"
#include "../../../Common/CropTable.h"
#include "../../../Common/SharedWallet.h"
#include "../../../Common/ShopLogic.h"
#include "../../../Shared/Protocol.h"
#include <optional>

// 월드에 배치되는 상호작용 객체의 종류 (Interactable_props_pos.json의 type과 같다)
enum class WorldObjectType { Field, House, Boat };

// 월드에 배치된 객체 하나. 위치·회전·크기는 JSON 값 그대로이며 클라이언트의 x·회전 y 반전은 적용하지 않았다
struct WorldObject {
	int objectId = -1; // 월드 객체 ID. 서버와 클라이언트가 같은 객체에 같은 번호를 쓴다
	WorldObjectType type = WorldObjectType::Field;
	Vec3f pos;
	Vec3f rot;
	Vec3f scale;
};

// 밭 하나의 서버 상태. objectId는 그 밭의 WorldObject와 같은 값이다
struct FieldState {
	int objectId = -1;
	CropState crop;
};

// 월드 객체 ID의 시작값. 플레이어 ID(세션 ID는 1부터)와 겹치지 않도록 파일에 적힌 순서대로 붙인다
const int WORLD_OBJECT_ID_BASE = 1000;

// 잠금을 해제한 뒤에도 로그와 디버거에서 확인할 수 있는 값의 사본이다.
struct FieldInteractResult {
    FieldInteractStatus status = FieldInteractStatus::PlayerNotReady;
    EItemID equippedTool = EItemID::Empty;
    CropState before;
    CropState after;
    int seedCountBefore = -1; // 씨앗을 선택하지 않은 경우 -1
    int seedCountAfter = -1;
    double distanceSquared = -1.0; // 거리 검사 전이면 -1
};

const char* GetFieldInteractStatusName(FieldInteractStatus status);

struct ShopTradeResult {
    ShopLogic::TradeResult trade;
    InventoryCounts counts;
    int balance;
};

// 서버의 게임 월드. 여러 스레드가 함께 쓰므로 읽거나 바꾸는 동안에는 Lock()/Unlock()으로 감싸야 한다
class GameWorld {
public:
	GameWorld() = default;
	GameWorld(const GameWorld&) = delete;
	GameWorld& operator=(const GameWorld&) = delete;

	// 월드를 쓰는 동안 다른 스레드가 접근하지 못하게 막는다. Lock()을 호출했으면 모든 경로에서 Unlock()을 호출해야 한다
	void Lock() { m_cs.Lock(); }
	void Unlock() { m_cs.Unlock(); }

	// 작물 데이터(crops.csv)를 읽어 월드를 준비한다. 실패하면 false
	bool Init(const char* cropCsvPath);

	// 접속한 플레이어의 개인 인벤토리를 만들고, 나가면 지운다
	void AddPlayer(int sessionId);
	void RemovePlayer(int sessionId);

	// 플레이어 조회
	PlayerState* GetPlayer(int sessionId);
	// 해당 플레이어의 인벤토리, 없으면 nullptr
	InventoryCore* GetInventory(int sessionId);
    // The caller holds the world lock while reading inventory counts.
    static InventoryCounts ReadInventoryCounts(const InventoryCore& inventory);

    // These methods lock the world and return a copy of the result and state.
    // nullopt means the player is absent or has not finished joining the world.
    std::optional<ShopTradeResult> BuySeed(int sessionId, EItemID seedId, int count);
    std::optional<ShopTradeResult> SellCrop(int sessionId, EItemID cropId, int count);

	// 빈 밭을 등록한다. 음수 ID나 중복 ID는 거부하며 기존 상태를 유지한다.
	bool AddField(int objectId);
	// 해당 밭의 상태, 없으면 nullptr. 조회만으로 밭을 만들지 않는다.
	FieldState* GetField(int objectId);

    // 위치·장비·밭·개인 수량을 검사하고 밭갈이 또는 파종을 처리한다. 임계영역을 잡은 채로 호출하며 송신은 하지 않는다.
    FieldInteractResult InteractField(int sessionId, int objectId);
	// 객체·밭·플레이어 초기 전송을 끝낸 ID 목록. 호출하는 쪽이 임계영역을 잡는다.
	std::vector<int> GetFieldSyncPlayers() const;

	// 월드에 하나뿐인 공동 지갑
	SharedWallet& GetWallet() { return m_wallet; }
	const CropTable& GetCropTable() const { return m_cropTable; }

	// 배치 데이터(JSON)를 읽어 월드 객체와 밭 상태를 등록한다. 스레드 시작 전에 한 번만 호출하며 실패하면 false
	bool LoadObjects(const char* objectsJsonPath);
	const std::vector<WorldObject>& GetObjects() const { return m_objects; }
	int GetFieldCount() const { return (int)m_fields.size(); }

	// 작물(당근·양배추)이 심겨 있는 밭인지
	bool HasCrop(int fieldId) const;

	// 월드의 까마귀. Lock()을 잡고 사용한다
	CrowSystem& GetCrowSystem() { return m_crowSystem; }
	const CrowSystem& GetCrowSystem() const { return m_crowSystem; }

	float GetDayTime() const { return m_timeSystem.GetDayTime();}
	// 낮(6~18시)인지
	bool IsDaytime() const { return m_timeSystem.IsDaytime(); }

	// 월드에 있는 플레이어들의 위치. Lock()을 잡고 호출한다
	std::vector<Vec3f> GetPlayerPositions() const;

	// 시간이 지난 만큼 월드를 갱신한다 (작물 성장, 까마귀 이동). dtMs는 밀리초. 호출하는 쪽이 Lock()을 잡아야 한다
	void Update(int dtMs);

private:
    enum class ShopAction { Buy, Sell };
    std::optional<ShopTradeResult> TradeShopItem(int sessionId, ShopAction action, EItemID itemId, int count);

	CriticalSection m_cs; // 월드 보호용 임계영역
	CropTable m_cropTable; // 작물 설정
	SharedWallet m_wallet; // 모든 플레이어가 함께 쓰는 돈
	std::unordered_map<int, PlayerState> m_players; // 세션 ID별 플레이어 상태
	std::unordered_map<int, FieldState> m_fields; // 월드 객체 ID별 밭 상태
	std::vector<WorldObject> m_objects; // 배치 데이터에서 읽은 월드 객체 (밭, 집, 보트)
	CrowSystem m_crowSystem; // 서버가 움직이는 까마귀

	TimeSystem m_timeSystem; // 공유하는 서버 게임 시간
};
