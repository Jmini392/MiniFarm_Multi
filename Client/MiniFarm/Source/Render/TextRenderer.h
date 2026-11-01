#pragma once
#include "Shader.h"

class Shader;

class TextRenderer
{
public:
    static void Init();

    static void Draw(const std::string& text,
        float centerX, float centerY,
        float scale,
        const glm::vec3& color);

    static void DrawLeft(const std::string& text,
        float x, float y,
        float scale,
        const glm::vec3& color);

    static void SetSpacing(float s) { s_spacing = s; }

private:
    static float CalcTextWidth(const std::string& text, float scale);

private:
    static GLuint s_VAO;
    static GLuint s_VBO;
    static GLuint s_texture;
    static std::shared_ptr<Shader> s_shader;

    static float s_spacing;
};
