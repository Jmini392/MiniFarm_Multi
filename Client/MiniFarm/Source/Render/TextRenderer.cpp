#include "PCH.h"
#include "TextRenderer.h"
#include "Shader.h"
#include "TextureLoader.h"

GLuint TextRenderer::s_VAO = 0;
GLuint TextRenderer::s_VBO = 0;
GLuint TextRenderer::s_texture = 0;
std::shared_ptr<Shader> TextRenderer::s_shader = nullptr;
float TextRenderer::s_spacing = 0.0f;

static const float GLYPH_W = 16.0f;
static const float GLYPH_H = 16.0f;
static const float TRIM_X = 0.30f;

void TextRenderer::Init()
{
    glGenVertexArrays(1, &s_VAO);
    glGenBuffers(1, &s_VBO);

    glBindVertexArray(s_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    s_shader = std::make_shared<Shader>("Shaders/text.vert", "Shaders/text.frag");

    glm::mat4 proj = glm::ortho(0.0f, (float)WINDOW_W,
        0.0f, (float)WINDOW_H);

    s_shader->Use();
    s_shader->SetProj(proj, "uProj");
    s_shader->SetTexture(0, "uFont");

    s_texture = TextureLoader::Load("Assets/font_atlas.png").id;
    LOG("TextRenderer loaded font atlas (tex = %u)", s_texture);
}

void TextRenderer::Draw(const std::string& text, float centerX, float centerY, float scale, const glm::vec3& color)
{
    float textWidth = CalcTextWidth(text, scale);
    float textHeight = GLYPH_H * scale;

    float startX = centerX - textWidth * 0.5f;
    float startY = centerY - textHeight * 0.5f;

    DrawLeft(text, startX, startY, scale, color);
}

void TextRenderer::DrawLeft(const std::string& text, float x, float y, float scale, const glm::vec3& color)
{
    if (!s_shader || s_texture == 0 || text.empty()) return;

    s_shader->Use();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform3f(glGetUniformLocation(s_shader->GetID(), "uTextColor"), color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_texture);

    glBindVertexArray(s_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_VBO);

    float cellU = 1.0f / 16.0f;
    float cellV = 1.0f / 16.0f;

    float usedRatio = 1.0f - (2.0f * TRIM_X);
    float drawW = GLYPH_W * scale * usedRatio;
    float drawH = GLYPH_H * scale;

    for (char c : text)
    {
        unsigned char uc = (unsigned char)c;

        int col = uc % 16;
        int rowTop = uc / 16;
        int row = 15 - rowTop;

        float u0 = (col + TRIM_X) * cellU;
        float u1 = (col + 1 - TRIM_X) * cellU;

        float v0 = row * cellV;
        float v1 = (row + 1) * cellV;

        float verts[6][4] = {
            { x,        y,        u0, v0 },
            { x,        y + drawH,  u0, v1 },
            { x + drawW,  y + drawH,  u1, v1 },

            { x,        y,        u0, v0 },
            { x + drawW,  y + drawH,  u1, v1 },
            { x + drawW,  y,        u1, v0 }
        };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += drawW + s_spacing;
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

float TextRenderer::CalcTextWidth(const std::string& text, float scale)
{
    float usedRatio = 1.0f - (2.0f * TRIM_X);
    float glyphW = GLYPH_W * scale * usedRatio;
    float adv = glyphW + s_spacing;

    if (text.empty()) return 0.f;

    return (float)text.size() * adv - s_spacing;
}
