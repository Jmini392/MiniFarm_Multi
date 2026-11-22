#include "PCH.h"
#include "Crow.h"
#include "GameWorld.h"

namespace {
	// 까마귀가 떠돌아다니는 구역 (플레이어 좌표). 밭(x -14~8, z -2~16)과 집 주변을 덮는다
	constexpr float CROW_WANDER_MIN_X = -16.f;
	constexpr float CROW_WANDER_MAX_X = 16.f;
	constexpr float CROW_WANDER_MIN_Z = -12.f;
	constexpr float CROW_WANDER_MAX_Z = 20.f;
	constexpr float CROW_WANDER_MIN_Y = 9.f;
	constexpr float CROW_WANDER_MAX_Y = 12.f;

	constexpr float CROW_WANDER_SPEED = 3.f;   // 배회 속도 (초당 거리)
	constexpr float CROW_APPROACH_SPEED = 5.f; // 밭으로 날아갈 때 속도
	constexpr int CROW_FIELD_SEARCH_MS = 1000; // 배회 중 작물 있는 밭을 찾는 간격

	// 플레이어를 알아채는 거리. 밤에는 어두워서 더 가까이 와야 알아챈다
	constexpr float CROW_NOTICE_DISTANCE_DAY = 7.f;
	constexpr float CROW_NOTICE_DISTANCE_NIGHT = 3.f;

	constexpr float CROW_FLEE_SPEED = 7.f;     // 도망갈 때 속도
	constexpr float CROW_FLEE_DISTANCE = 8.f;  // 플레이어 반대쪽으로 날아가는 수평 거리
	constexpr float CROW_FLEE_MIN_Y = 12.f;    // 도망가는 높이 (배회 높이보다 위)
	constexpr float CROW_FLEE_MAX_Y = 14.f;
	constexpr int CROW_FLEE_REST_MS = 3000;    // 도망친 뒤 다시 밭을 찾기까지 쉬는 시간

	// 밭 모델(field.obj)의 높이. 배치 y에 더하면 밭 윗면이 된다
	constexpr float FIELD_TOP_OFFSET = 2.f;

	float DistanceSquared(const Vec3f& a, const Vec3f& b) {
		const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
		return dx * dx + dy * dy + dz * dz;
	}

	// pos에서 가장 가까운 플레이어와 그 거리 제곱. 플레이어가 없으면 false
	bool FindNearestPlayer(const std::vector<Vec3f>& players, const Vec3f& pos, Vec3f& nearest, float& distSq) {
		bool found = false;
		for (const Vec3f& p : players) {
			const float d = DistanceSquared(pos, p);
			if (!found || d < distSq) {
				nearest = p;
				distSq = d;
				found = true;
			}
		}
		return found;
	}

	// 밭 윗면 위치 (플레이어 좌표). 밭이 아니면 false
	bool GetFieldLandingPos(const GameWorld& world, int fieldId, Vec3f& out) {
		// 월드 객체 ID는 파일 순서대로 붙으므로 바로 찾을 수 있다
		const std::vector<WorldObject>& objects = world.GetObjects();
		const int index = fieldId - WORLD_OBJECT_ID_BASE;
		if (index < 0 || index >= (int)objects.size()) return false;
		const WorldObject& obj = objects[index];
		if (obj.type != WorldObjectType::Field) return false;

		// 배치 원본의 X만 반전하면 플레이어 좌표가 된다
		out.x = -obj.pos.x;
		out.y = obj.pos.y + FIELD_TOP_OFFSET;
		out.z = obj.pos.z;
		return true;
	}
}

const char* GetCrowActionName(CrowAction action) {
	switch (action) {
	case CrowAction::Wander: return "Wander";
	case CrowAction::ApproachField: return "ApproachField";
	case CrowAction::OnField: return "OnField";
	case CrowAction::Flee: return "Flee";
	default: return "Unknown";
	}
}

int CrowSystem::FindFieldForCrow(const CrowState& crow, const GameWorld& world,
	const std::vector<Vec3f>& players, float noticeDistance) const {
	int best = -1;
	float bestDist = 0.f;
	for (const WorldObject& obj : world.GetObjects()) {
		const int fieldId = obj.objectId;
		if (obj.type != WorldObjectType::Field || !world.HasCrop(fieldId)) continue;

		// 다른 까마귀가 이미 노리는 밭은 건너뛴다
		bool taken = false;
		for (const CrowState& other : m_crows) {
			if (other.id != crow.id && other.targetFieldId == fieldId) { taken = true; break; }
		}
		if (taken) continue;

		Vec3f landing;
		if (!GetFieldLandingPos(world, fieldId, landing)) continue;

		// 플레이어가 지키고 있는 밭에는 가지 않는다 (가자마자 도망가는 것을 막는다)
		Vec3f player;
		float playerDistSq = 0.f;
		if (FindNearestPlayer(players, landing, player, playerDistSq)
			&& playerDistSq <= noticeDistance * noticeDistance) continue;

		const float dist = DistanceSquared(crow.pos, landing);
		if (best < 0 || dist < bestDist) {
			best = fieldId;
			bestDist = dist;
		}
	}
	return best;
}

Vec3f CrowSystem::PickWanderTarget() {
	std::uniform_real_distribution<float> rx(CROW_WANDER_MIN_X, CROW_WANDER_MAX_X);
	std::uniform_real_distribution<float> ry(CROW_WANDER_MIN_Y, CROW_WANDER_MAX_Y);
	std::uniform_real_distribution<float> rz(CROW_WANDER_MIN_Z, CROW_WANDER_MAX_Z);
	Vec3f p;
	p.x = rx(m_rng);
	p.y = ry(m_rng);
	p.z = rz(m_rng);
	return p;
}

Vec3f CrowSystem::PickFleeTarget(const Vec3f& crowPos, const Vec3f& playerPos) {
	// 플레이어에서 까마귀 쪽으로 향하는 수평 방향으로 날아간다
	float dx = crowPos.x - playerPos.x;
	float dz = crowPos.z - playerPos.z;
	float len = std::sqrt(dx * dx + dz * dz);
	if (len < 0.01f) {
		// 플레이어와 같은 자리면 아무 방향으로나 간다
		std::uniform_real_distribution<float> ra(0.f, 6.2831853f);
		const float a = ra(m_rng);
		dx = std::sin(a);
		dz = std::cos(a);
		len = 1.f;
	}

	std::uniform_real_distribution<float> ry(CROW_FLEE_MIN_Y, CROW_FLEE_MAX_Y);
	Vec3f p;
	p.x = std::clamp(crowPos.x + dx / len * CROW_FLEE_DISTANCE, CROW_WANDER_MIN_X, CROW_WANDER_MAX_X);
	p.y = ry(m_rng);
	p.z = std::clamp(crowPos.z + dz / len * CROW_FLEE_DISTANCE, CROW_WANDER_MIN_Z, CROW_WANDER_MAX_Z);
	return p;
}

void CrowSystem::Update(int dtMs, const GameWorld& world) {
	const std::vector<Vec3f> players = world.GetPlayerPositions();
	const bool isDay = world.IsDaytime();
	const float noticeDistance = isDay ? CROW_NOTICE_DISTANCE_DAY : CROW_NOTICE_DISTANCE_NIGHT;

	for (CrowState& crow : m_crows) {
		const bool goingToField = crow.action == CrowAction::ApproachField || crow.action == CrowAction::OnField;

		// 노리던 밭의 작물이 없어졌으면(수확 등) 다시 배회한다
		if (goingToField && !world.HasCrop(crow.targetFieldId)) {
			printf("[까마귀] ID=%d 밭 %d 작물 없음 -> 배회\n", crow.id, crow.targetFieldId);
			crow.action = CrowAction::Wander;
			crow.targetFieldId = -1;
			crow.target = PickWanderTarget();
			crow.searchCooldownMs = 0;
		}

		// 밭으로 가는 중이거나 앉아 있을 때 플레이어가 가까이 오면 상공으로 도망간다
		if (crow.action == CrowAction::ApproachField || crow.action == CrowAction::OnField) {
			Vec3f player;
			float distSq = 0.f;
			if (FindNearestPlayer(players, crow.pos, player, distSq)
				&& distSq <= noticeDistance * noticeDistance) {
				printf("[까마귀] ID=%d 플레이어 발견 (거리 %.1f, %s 기준 %.0f) -> 도주\n",
					crow.id, std::sqrt(distSq), isDay ? "낮" : "밤", noticeDistance);
				crow.action = CrowAction::Flee;
				crow.targetFieldId = -1;
				crow.target = PickFleeTarget(crow.pos, player);
			}
		}

		// 배회 중이면 일정 간격으로 작물이 있는 밭을 찾는다
		if (crow.action == CrowAction::Wander) {
			crow.searchCooldownMs -= dtMs;
			if (crow.searchCooldownMs <= 0) {
				crow.searchCooldownMs = CROW_FIELD_SEARCH_MS;
				const int fieldId = FindFieldForCrow(crow, world, players, noticeDistance);
				Vec3f landing;
				if (fieldId >= 0 && GetFieldLandingPos(world, fieldId, landing)) {
					crow.action = CrowAction::ApproachField;
					crow.targetFieldId = fieldId;
					crow.target = landing;
					printf("[까마귀] ID=%d 밭 %d로 접근 시작\n", crow.id, fieldId);
				}
			}
		}

		if (crow.action == CrowAction::OnField) continue; // 내려앉은 뒤에는 움직이지 않는다

		// 목표 지점을 향해 일정 속도로 날아간다
		float speed = CROW_WANDER_SPEED;
		if (crow.action == CrowAction::ApproachField) speed = CROW_APPROACH_SPEED;
		else if (crow.action == CrowAction::Flee) speed = CROW_FLEE_SPEED;
		const float step = speed * dtMs / 1000.f;
		const float dx = crow.target.x - crow.pos.x;
		const float dy = crow.target.y - crow.pos.y;
		const float dz = crow.target.z - crow.pos.z;
		const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

		// 수평으로 움직일 때만 방향을 바꾼다 (위아래로만 움직일 때 방향이 튀지 않게)
		if (dx * dx + dz * dz > 1e-6f) {
			crow.yaw = (float)(std::atan2(dx, dz) * 180.0 / 3.14159265358979);
		}

		if (dist > step) {
			crow.pos.x += dx / dist * step;
			crow.pos.y += dy / dist * step;
			crow.pos.z += dz / dist * step;
			continue;
		}

		// 도착
		crow.pos = crow.target;
		if (crow.action == CrowAction::ApproachField) {
			crow.action = CrowAction::OnField;
			printf("[까마귀] ID=%d 밭 %d에 도착 (%.1f, %.1f, %.1f)\n", crow.id, crow.targetFieldId, crow.pos.x, crow.pos.y, crow.pos.z);
		}
		else if (crow.action == CrowAction::Flee) {
			// 상공까지 도망쳤으면 잠시 쉬었다가 다시 밭을 찾는다
			crow.action = CrowAction::Wander;
			crow.target = PickWanderTarget();
			crow.searchCooldownMs = CROW_FLEE_REST_MS;
			printf("[까마귀] ID=%d 도주 끝 (%.1f, %.1f, %.1f) -> 배회\n", crow.id, crow.pos.x, crow.pos.y, crow.pos.z);
		}
		else {
			crow.target = PickWanderTarget();
		}
	}
}

int CrowSystem::AddCrow(const Vec3f& pos) {
	if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z)) {
		return -1;
	}

	CrowState crow;
	crow.id = CROW_ID_BASE + (int)m_crows.size();
	crow.pos = pos;
	crow.target = PickWanderTarget();
	m_crows.push_back(crow);

	return crow.id;
}

CrowState* CrowSystem::GetCrow(int crowId) {
	// 까마귀는 지워지지 않으므로 ID에서 순서를 바로 알 수 있다
	const int index = crowId - CROW_ID_BASE;
	if (index < 0 || index >= (int)m_crows.size()) {
		return nullptr;
	}

	return &m_crows[index];
}
