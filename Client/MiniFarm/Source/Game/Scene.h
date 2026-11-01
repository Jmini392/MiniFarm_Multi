#pragma once
#include "GameObject.h"
#include "Camera.h"

class Scene
{
public:
    virtual void Init() = 0;
    virtual void Update(int dt) = 0;
    virtual void Draw() = 0;
    virtual ~Scene() {}

    Camera& GetCamera() { return m_camera; }
    std::vector<std::shared_ptr<GameObject>>& GetObjects() { return m_objects; }
    void AddObject(std::shared_ptr<GameObject> obj)
    {
        m_objects.push_back(obj);
    }
    glm::vec3 GetClearColor() const { return m_clearColor; }
    void SetClearColor(const glm::vec3& color) { m_clearColor = color; }

    bool IsValid() { return m_valid; }
    void SetValid(bool valid) { m_valid = valid; }
protected:
    bool m_valid = false;
    Camera m_camera;
    std::vector<std::shared_ptr<GameObject>> m_objects;
    glm::vec3 m_clearColor = glm::vec3(0.f);
};
