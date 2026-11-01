#pragma once
class Shader
{
public:
	Shader() = default;
	Shader(const char* vertexPath, const char* fragmentPath)
	{
		Init(vertexPath, fragmentPath);
	}
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = default;
	Shader& operator=(Shader&&) = default;

	void Init(const char* vertexPath, const char* fragmentPath);
	void Use() { glUseProgram(m_programID); }
	GLuint GetID() { return m_programID; }

	void SetModel(const glm::mat4& m_model, const char* name = "uModel");
	void SetView(const glm::mat4& view, const char* name = "uView");
	void SetProj(const glm::mat4& proj, const char* name = "uProj");
	void SetTexture(GLint textureUnit, const char* name = "atexture");

	void SetLightPos(const glm::vec3& pos, const char* name = "uLightPos");
	void SetLightColor(const glm::vec3& color, const char* name = "uLightColor");
	void SetViewPos(const glm::vec3& pos, const char* name = "uViewPos");

	void SetVec2(const glm::vec2& v, const char* name);
	void SetVec3(const glm::vec3& v, const char* name);
	void SetVec4(const glm::vec4& v, const char* name);
	void SetFloat(float v, const char* name);
	void SetInt(int v, const char* name);

	void BeginDebugDraw(const glm::vec3& color);
	void EndDebugDraw();
private:
	GLuint m_programID = 0;
	GLuint CompileShader(const std::string& source, GLenum type);
};
