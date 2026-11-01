#include "PCH.h"
#include "UIRenderer.h"
#include "Shader.h"
#include "TextRenderer.h"

GLuint UIRenderer::s_VAO = 0;
GLuint UIRenderer::s_VBO = 0;
std::shared_ptr<Shader> UIRenderer::s_shader = nullptr;

void UIRenderer::Init()
{
	// Fullscreen quad vertices
	float vertices[] = {
		// pos        // uv
		0.0f, 0.0f,   0.0f, 0.0f,
		1.0f, 0.0f,   1.0f, 0.0f,
		1.0f, 1.0f,   1.0f, 1.0f,

		0.0f, 0.0f,   0.0f, 0.0f,
		1.0f, 1.0f,   1.0f, 1.0f,
		0.0f, 1.0f,   0.0f, 1.0f,
	};

	glGenVertexArrays(1, &s_VAO);
	glGenBuffers(1, &s_VBO);

	glBindVertexArray(s_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, s_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// aPos
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// aUV
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
		(void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	s_shader = std::make_shared<Shader>("Shaders/ui.vert", "Shaders/ui.frag");

	if (s_shader->GetID() == 0)
	{
		LOG_E("Text shader program is 0 (compile/link failed)");
	}
}

void UIRenderer::Draw(const TextureInfo& textureInfo, const glm::vec2& posNDC, float size, const glm::vec4& color)
{
	if (!s_shader) return;
	float aspect = (float)textureInfo.width / (float)textureInfo.height;
	glm::vec2 sizeNDC(size, size / aspect);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	s_shader->Use();
	s_shader->SetVec2(posNDC, "uPos");
	s_shader->SetVec2(sizeNDC, "uSize");
	s_shader->SetVec4(color, "uColor");
	s_shader->SetFloat(1.0f, "uMax");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureInfo.id);

	glBindVertexArray(s_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void UIRenderer::DrawCenter(const TextureInfo& textureInfo, const glm::vec2& centerNDC, float size, const glm::vec4& color)
{
	glm::vec2 posNDC = centerNDC - glm::vec2(size / 2.0f, size / 2.0f * ((float)textureInfo.height / (float)textureInfo.width));
	Draw(textureInfo, posNDC, size, color);
}

void UIRenderer::DrawMessage(const std::string& msg)
{
	UIRenderer::DrawCenter(TextureLoader::Load("Assets/ui_board_black.png"), { 0.5f,0.5f }, 0.6f, { 1,1,1,0.7 });
	std::stringstream ss(msg);
	std::vector<std::string> lines;
	std::string line;

	while (std::getline(ss, line, '\n'))
		lines.push_back(line);

	float lineHeight = 30.f;

	float totalHeight = lineHeight * (lines.size() - 1);
	float startY = WINDOW_H / 2 - totalHeight * 0.5f;

	for (int i = 0; i < lines.size(); i++)
	{
		float y = startY + i * lineHeight;
		TextRenderer::Draw(lines[lines.size() - 1 - i], WINDOW_W / 2, y, 2, glm::vec4(1.f));
	}
}

void UIRenderer::DrawFill(const TextureInfo& textureInfo,
	const glm::vec2& posNDC,
	float size,
	float progress,
	const glm::vec4& color)
{
	if (!s_shader) return;

	float aspect = (float)textureInfo.width / (float)textureInfo.height;
	glm::vec2 sizeNDC(size, size / aspect);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	s_shader->Use();
	s_shader->SetVec2(posNDC, "uPos");
	s_shader->SetVec2(sizeNDC, "uSize");
	s_shader->SetVec4(color, "uColor");

	s_shader->SetFloat(progress, "uMax");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureInfo.id);

	glBindVertexArray(s_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

