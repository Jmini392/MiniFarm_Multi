#pragma once
class GameObject;
struct ObjectInfo;

class ObjectFactory
{
public:
    static std::shared_ptr<GameObject> Create(
        const std::string& type,
        const glm::vec3& pos,
        const glm::vec3& rot,
        const glm::vec3& scale,
        const ObjectInfo* info
    );
};