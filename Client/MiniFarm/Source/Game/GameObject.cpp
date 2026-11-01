#include "PCH.H"
#include "GameObject.h"
#include "Shader.h"
#include "Collider.h"
#include "DebugDrawer.h"
#include "Model.h"
#include "BoxCollider.h"
#include "SceneManager.h"

void GameObject::Init()
{
    if (m_model)
    {
        glm::vec3 min = m_model->GetMinBounds();
        glm::vec3 max = m_model->GetMaxBounds();

        m_collider = std::make_shared<BoxCollider>(min, max);
        m_collider->UpdatePos(m_pos);
        m_collider->InitTransform(m_pos, m_scale, m_rot);
    }
}

void GameObject::Update(int time)
{
    if (!m_alive) return;
    if (m_collider)
    {
        m_collider->UpdatePos(m_pos);
        m_collider->InitTransform(m_pos, m_scale, m_rot);
    }
}

void GameObject::Draw()
{
    if (!m_model) return;

    glm::mat4 modelMat =
        Translate(m_pos)
        * Rotate(m_rot.y, { 0,1,0 })
        * Rotate(m_rot.x, { 1,0,0 })
        * Rotate(m_rot.z, { 0,0,1 })
        * Scale(m_scale);

    SceneManager::GetMainShader().SetModel(modelMat);
    m_model->Draw();
}

void GameObject::DebugDraw()
{
    if (m_collider)
        m_collider->DrawDebug();
}