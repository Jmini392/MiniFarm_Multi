#pragma once
#include "Collider.h"

class BoxCollider : public Collider
{
public:
	BoxCollider(const glm::vec3& min, const glm::vec3& max);

	virtual void DrawDebug() override;
	void InitTransform(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot) override;
	void UpdatePos(const glm::vec3& pos) override;
	bool Intersects(const Collider* other) const;
	bool IntersectSegment(const glm::vec3& start, const glm::vec3& end,
		glm::vec3& hitPoint, float* outT) const override;

	bool Raycast(const glm::vec3& origin,
		const glm::vec3& dir,
		float& outT,
		glm::vec3& outHit) const override;
public:
	glm::vec3 m_localMin;
	glm::vec3 m_localMax;

	glm::vec3 m_worldMin;
	glm::vec3 m_worldMax;
};
