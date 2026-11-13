#pragma once
#include "Common.h"
#include <unordered_map>
#include <vector>
#include "FieldState.h"
#include "PlayerState.h"
#include "../../../Common/CropTable.h"
#include "../../../Common/InventoryCore.h"
#include "../../../Common/SharedWallet.h"

// 월드에 배치되는 상호작용 객체의 종류 (Interactable_props_pos.json의 type과 같다)
enum class WorldObjectType { Field, House, Boat };

struct Vec3f {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

// 월드에 배치된 객체 하나
// 위치·회전·크기는 JSON에 적힌 값 그대로이다.
// (클라이언트가 파일을 읽어 그릴 때 하던 x 반전, 회전 y 반전은 적용하지 않았다)
struct WorldObject {
	int objectId = -1; // 월드 객체 ID. 서버와 클라이언트가 같은 객체에 같은 번호를 쓴다
	WorldObjectType type = WorldObjectType::Field;
	Vec3f pos;
	Vec3f rot;
	Vec3f scale;
};

// 월드 객체 ID는 이 값부터 파일에 적힌 순서대로 붙는다
// 플레이어 ID(세션 ID는 1부터)와 겹치지 않게 하려는 것이다
const int WORLD_OBJECT_ID_BASE = 1000;

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
	// 객체·밭·플레이어 초기 전송을 끝낸 ID 목록. 호출하는 쪽이 Lock을 잡는다.
	std::vector<int> GetFieldSyncPlayers() const;

	// 월드에 하나뿐인 공동 지갑
	SharedWallet& GetWallet() { return m_wallet; }
	const CropTable& GetCropTable() const { return m_cropTable; }
	int GetPlayerCount() const { return (int)m_players.size(); }

	// 배치 데이터(JSON)를 읽어 월드 객체를 등록한다. 밭은 FieldState도 함께 만든다. 실패하면 false
	// 스레드를 시작하기 전에 한 번만 호출한다. 이후 객체 목록은 바뀌지 않으므로 읽을 때 Lock이 필요 없다
	bool LoadObjects(const char* objectsJsonPath);
	const std::vector<WorldObject>& GetObjects() const { return m_objects; }
	int GetFieldCount() const { return (int)m_fields.size(); }

	// 시간이 지난 만큼 월드를 갱신한다 (밭의 작물 성장). dtMs는 밀리초. 호출하는 쪽이 Lock을 잡아야 한다
	void Update(int dtMs);

	// TODO: 상점 거래 같은 집·보트 상호작용 처리

private:
	CRITICAL_SECTION m_cs; // 월드 보호용 임계영역
	CropTable m_cropTable; // 작물 설정
	SharedWallet m_wallet; // 모든 플레이어가 함께 쓰는 돈
	std::unordered_map<int, PlayerState> m_players; // 세션 ID별 플레이어 상태
	std::unordered_map<int, FieldState> m_fields; // 월드 객체 ID별 밭 상태
	std::vector<WorldObject> m_objects; // 배치 데이터에서 읽은 월드 객체 (밭, 집, 보트)
};
