#include "PCH.h"
#include "BoxCollider.h"
#include "Shader.h"
#include "DebugDrawer.h"

BoxCollider::BoxCollider(const glm::vec3& min, const glm::vec3& max)
	: m_localMin(min), m_localMax(max)
{
	m_worldMin = min;
	m_worldMax = max;
}

void BoxCollider::InitTransform(const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& rot)
{
	glm::vec3 localCenter = (m_localMax + m_localMin) * 0.5f;
	glm::vec3 localHalf = (m_localMax - m_localMin) * 0.5f * scale;

	glm::mat4 R = glm::yawPitchRoll(
		glm::radians(rot.y),
		glm::radians(rot.x),
		glm::radians(rot.z)
	);

	glm::mat3 rotMat = glm::mat3(R);

	glm::vec3 corners[8] = {
		localCenter + glm::vec3(-localHalf.x, -localHalf.y, -localHalf.z),
		localCenter + glm::vec3(localHalf.x, -localHalf.y, -localHalf.z),
		localCenter + glm::vec3(localHalf.x, -localHalf.y,  localHalf.z),
		localCenter + glm::vec3(-localHalf.x, -localHalf.y,  localHalf.z),
		localCenter + glm::vec3(-localHalf.x,  localHalf.y, -localHalf.z),
		localCenter + glm::vec3(localHalf.x,  localHalf.y, -localHalf.z),
		localCenter + glm::vec3(localHalf.x,  localHalf.y,  localHalf.z),
		localCenter + glm::vec3(-localHalf.x,  localHalf.y,  localHalf.z),
	};

	glm::vec3 wMin(99999.f), wMax(-99999.f);

	for (int i = 0; i < 8; i++)
	{
		glm::vec3 w = rotMat * corners[i] + pos;

		wMin = glm::min(wMin, w);
		wMax = glm::max(wMax, w);
	}

	m_worldMin = wMin;
	m_worldMax = wMax;
}

void BoxCollider::UpdatePos(const glm::vec3& pos)
{
	glm::vec3 half = (m_localMax - m_localMin) * 0.5f;
	glm::vec3 center = (m_localMax + m_localMin) * 0.5f;

	glm::vec3 worldCenter = center + pos;

	m_worldMin = worldCenter - half;
	m_worldMax = worldCenter + half;
}

bool BoxCollider::Intersects(const Collider* other) const
{
	const BoxCollider* box = dynamic_cast<const BoxCollider*>(other);
	if (!box) return false;

	// AABB intersects?
	return !(m_worldMax.x < box->m_worldMin.x ||
		m_worldMin.x > box->m_worldMax.x ||
		m_worldMax.y < box->m_worldMin.y ||
		m_worldMin.y > box->m_worldMax.y ||
		m_worldMax.z < box->m_worldMin.z ||
		m_worldMin.z > box->m_worldMax.z);
}

bool BoxCollider::IntersectSegment(const glm::vec3& start, const glm::vec3& end, glm::vec3& hitPoint, float* outT) const
{
	float t;
	glm::vec3 dir = end - start;
	glm::vec3 invDir = 1.0f / dir;

	float tMin = 0.0f;
	float tMax = 1.0f;  // segment range is 0~1

	for (int i = 0; i < 3; i++)
	{
		float t1 = (m_worldMin[i] - start[i]) * invDir[i];
		float t2 = (m_worldMax[i] - start[i]) * invDir[i];

		float tNear = glm::min(t1, t2);
		float tFar = glm::max(t1, t2);

		tMin = glm::max(tMin, tNear);
		tMax = glm::min(tMax, tFar);

		if (tMax < tMin)
			return false; // no collision
	}

	*outT = tMin;
	hitPoint = start + dir * tMin;
	return true;
}

bool BoxCollider::Raycast(const glm::vec3& origin, const glm::vec3& dir, float& outT, glm::vec3& outHit) const
{
	float tMin = 0.0f;
	float tMax = 10000.0f; // 충분히 큰 값

	glm::vec3 invDir = 1.0f / dir;

	for (int i = 0; i < 3; i++)
	{
		float t1 = (m_worldMin[i] - origin[i]) * invDir[i];
		float t2 = (m_worldMax[i] - origin[i]) * invDir[i];

		float tNear = glm::min(t1, t2);
		float tFar = glm::max(t1, t2);

		tMin = glm::max(tMin, tNear);
		tMax = glm::min(tMax, tFar);

		if (tMax < tMin)
			return false;
	}

	outT = tMin;
	outHit = origin + dir * tMin;
	return true;
}

void BoxCollider::DrawDebug()
{
	DebugDrawer::DrawBox(m_worldMin, m_worldMax, glm::vec3(1, 0, 0));
}
