#pragma once
#include <unordered_map>
#include "../../../Common/CropTable.h"
#include "../../../Common/InventoryCore.h"
#include "../../../Common/SharedWallet.h"

// 서버가 가지고 있는 게임 월드
// 공용 로직(Common)이 변경할 상태를 이곳에서 가진다
// 아직 동기화하지 않았으므로 스레드를 쓰게 되면 임계영역으로 보호해야 한다
class GameWorld {
public:
	// 작물 데이터(crops.csv)를 읽어 월드를 준비한다. 실패하면 false
	bool Init(const char* cropCsvPath);

	// 접속한 플레이어의 개인 인벤토리를 만들고, 나가면 지운다
	void AddPlayer(int sessionId);
	void RemovePlayer(int sessionId);

	// 해당 플레이어의 인벤토리, 없으면 nullptr
	InventoryCore* GetInventory(int sessionId);

	// 월드에 하나뿐인 공동 지갑
	SharedWallet& GetWallet() { return m_wallet; }
	const CropTable& GetCropTable() const { return m_cropTable; }
	int GetPlayerCount() const { return (int)m_inventories.size(); }

	// TODO: 밭, 상점, 보트 같은 월드 오브젝트 관리와 배치 데이터 로딩

private:
	CropTable m_cropTable; // 작물 설정
	SharedWallet m_wallet; // 모든 플레이어가 함께 쓰는 돈
	std::unordered_map<int, InventoryCore> m_inventories; // 세션 ID별 개인 인벤토리
};
