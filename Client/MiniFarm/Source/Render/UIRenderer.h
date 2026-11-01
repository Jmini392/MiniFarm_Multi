#pragma once
#include "TextureLoader.h"

class Shader;
class UIRenderer
{
public:
	static void Init();
	static void Draw(const TextureInfo& textureInfo, const glm::vec2& posNDC, float size, const glm::vec4& color = glm::vec4(1.0f));
	static void DrawCenter(const TextureInfo& textureInfo, const glm::vec2& centerNDC, float size, const glm::vec4& color = glm::vec4(1.0f));
	static void DrawMessage(const std::string& msg);
	static void DrawFill(const TextureInfo& textureInfo, const glm::vec2& posNDC, float size, float progress, const glm::vec4& color);
private:
	static GLuint s_VAO, s_VBO;
	static std::shared_ptr<Shader> s_shader;
};
