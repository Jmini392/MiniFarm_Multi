#pragma once
#include <random>

class GameWorld;

// 까마귀 ID는 이 값부터 만든 순서대로 붙는다. 플레이어(1~)와 월드 객체(1000~)와 겹치지 않는다
const int CROW_ID_BASE = 2000;

// 까마귀 모델 크기 배율. 원래 모델(높이 1.7)이 플레이어만 해서 조금 줄인다
const float CROW_SCALE = 0.7f;

// 까마귀가 지금 하고 있는 일
enum class CrowAction {
	Wander,        // 맵 위를 떠돌아다닌다
	ApproachField, // 작물이 있는 밭으로 날아가는 중
	OnField,       // 밭에 내려앉았다 (쪼아 먹기는 이후 작업)
	Flee           // 플레이어를 보고 상공으로 도망가는 중. 끝나면 다시 배회한다
};

const char* GetCrowActionName(CrowAction action);

// 맵을 날아다니는 까마귀 하나. 위치는 플레이어와 같은 월드 좌표(JSON 값 그대로가 아님)이다
struct CrowState {
	int id = -1;
	Vec3f pos;
	float yaw = 0.f; // 바라보는 방향(도). 플레이어와 같이 atan2(x, z) 기준이다
	CrowAction action = CrowAction::Wander;
	Vec3f target;             // 지금 날아가는 목표 지점
	int targetFieldId = -1;   // ApproachField·OnField일 때 노리는 밭의 객체 ID
	int searchCooldownMs = 0; // Wander 중 다음 밭 탐색까지 남은 시간 (도망친 직후에는 더 길다)
};

// 월드의 까마귀 전체를 가지고 움직인다. GameWorld가 하나 가지며, 모든 함수는 월드 Lock()을 잡은 채 호출한다
class CrowSystem {
public:
	// 까마귀를 만들고 ID를 돌려준다. 좌표가 유한한 값이 아니면 -1. 송신은 하지 않는다
	int AddCrow(const Vec3f& pos);
	// 해당 까마귀, 없으면 nullptr
	CrowState* GetCrow(int crowId);
	const std::vector<CrowState>& GetCrows() const { return m_crows; }

	// 까마귀를 배회시키거나 작물이 있는 밭으로 날려 보내고, 플레이어가 다가오면 도망가게 한다. world는 읽기만 한다
	void Update(int dtMs, const GameWorld& world);

private:
	// 다른 까마귀가 노리지 않고 근처에 플레이어도 없는, 작물이 있는 밭 중 가장 가까운 것. 없으면 -1
	int FindFieldForCrow(const CrowState& crow, const GameWorld& world,
		const std::vector<Vec3f>& players, float noticeDistance) const;
	// 배회 구역 안에서 다음 목표 지점을 고른다
	Vec3f PickWanderTarget();
	// 플레이어 반대쪽 상공에서 도망갈 지점을 고른다
	Vec3f PickFleeTarget(const Vec3f& crowPos, const Vec3f& playerPos);

	std::vector<CrowState> m_crows;
	std::mt19937 m_rng{ std::random_device{}() }; // 배회 지점용 난수
};
