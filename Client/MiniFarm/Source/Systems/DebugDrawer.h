#pragma once

class Shader;
class DebugDrawer
{
public:
    static void Init(Shader* shader);
    static void DrawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);
    static void DrawArrow(const glm::vec3& pos, const glm::vec3& dir, float len, const glm::vec3& color);
    static void DrawAxis(const glm::vec3& pos, float size);
    static void DrawBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
private:
    static Shader* s_shader;
};
