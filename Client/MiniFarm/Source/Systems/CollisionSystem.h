#pragma once

class GameObject;
class CollisionSystem
{
public:
    static glm::vec3 TryMove(GameObject* obj, const glm::vec3& desiredPos);
    static void CheckCollisions();

    static bool IsVerticalObstacle(GameObject* obj, GameObject* other);
    static bool CheckSupportBelow(GameObject* obj, const glm::vec3& position, float& supportY);
};