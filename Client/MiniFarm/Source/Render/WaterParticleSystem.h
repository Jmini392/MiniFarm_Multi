#pragma once

class Shader;

struct WaterParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    float maxLife;
    float size;
};

class WaterParticleSystem {
public:
    static void Init();
    static void Update(float dt);
    static void Draw(Shader* mainShader);
    static void Watering(const glm::vec3& centerPos);

private:
    static void UpdateBuffers();

    static constexpr size_t PARTICLES_PER_WATERING = 30;
    static constexpr size_t MAX_PARTICLES = PARTICLES_PER_WATERING * 8;

    static GLuint s_vao;
    static GLuint s_vbo;
    static GLuint s_shaderProgram;
    static std::vector<WaterParticle> s_particles;
    static float s_gravity;
};