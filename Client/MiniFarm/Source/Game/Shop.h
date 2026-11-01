#pragma once
#include "InteractableObject.h"
#include "GameTypes.h"

class Shop : public InteractableObject
{
public:
	Shop(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale,
		const std::string& model, const std::string& texture)
		: InteractableObject(pos, rot, scale, 9.0f)
	{
		m_model = ModelCache::Get(model, texture);
	}
	virtual void OnInteract(Player* player) override;

	void Buy(Player* player);
	void Sell(Player* player);

	void SetCropType(EItemID cropType) { m_cropType = cropType; }
private:
	EItemID m_cropType = EItemID::Empty;
};

