#pragma once

class Shader;
class Model;

class ModelCache
{
public:
	static std::shared_ptr<Model> Get(const std::string& modelPath,
		const std::string& texturePath);

private:
	static std::unordered_map<std::string, std::shared_ptr<Model>> s_cache;
};

class Model
{
public:
	Model() = default;
	Model(const std::string& name);
	Model(const std::string& path, const std::string& texturePath);
	~Model();

	void Draw();
	void LoadOBJ(const std::string& path);
	void SetupBuffers();
	void InitBounds();
	glm::vec3 GetMinBounds() const { return m_minBounds; }
	glm::vec3 GetMaxBounds() const { return m_maxBounds; }

private:
	struct Vertex {
		glm::vec3 m_pos;
		glm::vec3 m_color;
		glm::vec3 m_normal;
		glm::vec2 m_uv;
	};
 
	std::vector<Vertex> m_vertices;
	GLuint m_vao = 0, m_vbo = 0;
	GLuint m_textureID = 0;

	glm::vec3 m_minBounds {0};
	glm::vec3 m_maxBounds {0};
};
