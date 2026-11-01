#pragma once
#include "GameObject.h"


class StaticProp : public GameObject
{
public:
	std::string m_modelPath;
	std::string m_texturePath;

	StaticProp(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale,
		const std::string& model, const std::string& texture)
		: GameObject(pos, rot, scale), m_modelPath(model), m_texturePath(texture)
	{
		m_model = ModelCache::Get(m_modelPath, m_texturePath);
	}
};