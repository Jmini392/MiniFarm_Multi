#pragma once
#include "GameObject.h"
#include "IInteractable.h"

class InteractableObject : public GameObject, public IInteractable
{
public:
	InteractableObject(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale
		, float interactDistance = 2.5f)
		: GameObject(pos, rot, scale), m_interactDistance(interactDistance)
	{

	}

	virtual bool IsInteractable() const override { return true; }
	virtual float GetInteractDistance() const override
	{
		return m_interactDistance;
	}

protected:
	float m_interactDistance;
};