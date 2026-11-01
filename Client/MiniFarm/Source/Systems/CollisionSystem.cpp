#include "PCH.h"
#include "CollisionSystem.h"
#include "SceneManager.h"
#include "Collider.h"
#include "BoxCollider.h"

glm::vec3 CollisionSystem::TryMove(GameObject* obj, const glm::vec3& desiredPos)
{
    auto& objects = SceneManager::GetObjects();

    if (!obj->m_collider)
        return desiredPos;

    glm::vec3 oldPos = obj->m_pos;
    glm::vec3 newPos = desiredPos;

    obj->m_pos = glm::vec3(desiredPos.x, oldPos.y, oldPos.z);
    obj->m_collider->UpdatePos(obj->m_pos);

    for (auto& other : objects)
    {
        if (other.get() == obj || !other->m_collider) continue;    
        if (IsVerticalObstacle(obj, other.get()) && obj->m_collider->Intersects(other->m_collider.get()))
        {
            newPos.x = oldPos.x;
            break;
        }
    }

    obj->m_pos = glm::vec3(newPos.x, oldPos.y, desiredPos.z);
    obj->m_collider->UpdatePos(obj->m_pos);

    for (auto& other : objects)
    {
        if (other.get() == obj || !other->m_collider) continue;
        if (IsVerticalObstacle(obj, other.get()) && obj->m_collider->Intersects(other->m_collider.get()))
        {
            newPos.z = oldPos.z;
            break;
        }
    }

    obj->m_pos = oldPos;
    return newPos;
}

void CollisionSystem::CheckCollisions()
{
    auto& objects = SceneManager::GetObjects();

    for (auto& obj : objects)
    {
        if (!obj->m_collider) continue;

        for (auto& other : objects)
        {
            if (other.get() == obj.get()) continue;
            if (!other->m_collider) continue;

            if (obj->m_collider->Intersects(other->m_collider.get()))
            {
                obj->OnCollision(other.get());
            }
        }
    }
}

bool CollisionSystem::IsVerticalObstacle(GameObject* obj, GameObject* other)
{
    BoxCollider* objBoxCollider = dynamic_cast<BoxCollider*>(obj->m_collider.get());
    BoxCollider* otherBoxCollider = dynamic_cast<BoxCollider*>(other->m_collider.get());

    if (!objBoxCollider || !otherBoxCollider)
        return true;

    glm::vec3 objMin = objBoxCollider->m_worldMin;
    glm::vec3 objMax = objBoxCollider->m_worldMax;
    glm::vec3 otherMin = otherBoxCollider->m_worldMin;
    glm::vec3 otherMax = otherBoxCollider->m_worldMax;

    float groundTolerance = 0.3f;
    float heightDifference = objMin.y - otherMax.y;

    if (heightDifference >= -groundTolerance && heightDifference <= groundTolerance)
    {
        return false;
    }

    float ceilingDifference = otherMin.y - objMax.y;
    if (ceilingDifference >= -groundTolerance && ceilingDifference <= groundTolerance)
    {
        return false;
    }

    return true;
}

bool CollisionSystem::CheckSupportBelow(GameObject* obj, const glm::vec3& position, float& supportY)
{
    auto& objects = SceneManager::GetObjects();
    
    BoxCollider* objBoxCollider = dynamic_cast<BoxCollider*>(obj->m_collider.get());
    
    glm::vec3 originalPos = obj->m_pos;
    obj->m_pos = position;
    objBoxCollider->UpdatePos(position);

    glm::vec3 objMin = objBoxCollider->m_worldMin;
    glm::vec3 objMax = objBoxCollider->m_worldMax;

    float playerBottomY = objMin.y;
    float closestSupportY = -std::numeric_limits<float>::max();
    bool foundSupport = false;

    for (auto& other : objects)
    {
        if (other.get() == obj) continue;
        if (!other->m_collider) continue;

        BoxCollider* otherBoxCollider = dynamic_cast<BoxCollider*>(other->m_collider.get());
        if (!otherBoxCollider) continue;

        glm::vec3 otherMin = otherBoxCollider->m_worldMin;
        glm::vec3 otherMax = otherBoxCollider->m_worldMax;

        if (objMax.x > otherMin.x && objMin.x < otherMax.x && objMax.z > otherMin.z && objMin.z < otherMax.z)
        {
            float objectTopY = otherMax.y;

            if (playerBottomY >= objectTopY - 0.1f && playerBottomY <= objectTopY + 0.1f)
            {
                if (objectTopY > closestSupportY)
                {
                    closestSupportY = objectTopY;
                    foundSupport = true;
                }
            }
        }
    }

    obj->m_pos = originalPos;
    objBoxCollider->UpdatePos(originalPos);

    if (foundSupport)
    {
        supportY = closestSupportY;
        return true;
    }

    return false;
}