#pragma once
#include "InteractableObject.h"

class Boat : public InteractableObject
{
public:
	Boat(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale,
		const std::string& model, const std::string& texture)
		: InteractableObject(pos, rot, scale, 20.0f)
	{
		m_model = ModelCache::Get(model, texture);
	}
	virtual void OnInteract(Player* player) override;
};

