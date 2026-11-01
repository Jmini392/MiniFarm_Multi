#include "PCH.h"
#include "WaterParticleSystem.h"
#include "Shader.h"
#include <random>

GLuint WaterParticleSystem::s_vao = 0;
GLuint WaterParticleSystem::s_vbo = 0;
GLuint WaterParticleSystem::s_shaderProgram = 0;
std::vector<WaterParticle> WaterParticleSystem::s_particles;
size_t WaterParticleSystem::s_activeParticles = 0;
float WaterParticleSystem::s_gravity = 8.f;

void WaterParticleSystem::Init() {
    s_particles.resize(MAX_PARTICLES);

    std::string vertSource = LoadFile("Shaders/particle.vert");
    if (vertSource.empty()) {
        LOG_E("Failed to load particle.vert");
        return;
    }
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertSrc = vertSource.c_str();
    glShaderSource(vertShader, 1, &vertSrc, nullptr);
    glCompileShader(vertShader);

    GLint success;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar log[512];
        glGetShaderInfoLog(vertShader, 512, nullptr, log);
        LOG_E("Vertex shader compile failed: %s", log);
    }

    std::string geomSource = LoadFile("Shaders/particle.geom");
    if (geomSource.empty()) {
        LOG_E("Failed to load particle.geom");
        return;
    }
    GLuint geomShader = glCreateShader(GL_GEOMETRY_SHADER);
    const char* geomSrc = geomSource.c_str();
    glShaderSource(geomShader, 1, &geomSrc, nullptr);
    glCompileShader(geomShader);

    glGetShaderiv(geomShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar log[512];
        glGetShaderInfoLog(geomShader, 512, nullptr, log);
        LOG_E("Geometry shader compile failed: %s", log);
    }

    std::string fragSource = LoadFile("Shaders/particle.frag");
    if (fragSource.empty()) {
        LOG_E("Failed to load particle.frag");
        return;
    }
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragSrc = fragSource.c_str();
    glShaderSource(fragShader, 1, &fragSrc, nullptr);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar log[512];
        glGetShaderInfoLog(fragShader, 512, nullptr, log);
        LOG_E("Fragment shader compile failed: %s", log);
    }

    s_shaderProgram = glCreateProgram();
    glAttachShader(s_shaderProgram, vertShader);
    glAttachShader(s_shaderProgram, geomShader);
    glAttachShader(s_shaderProgram, fragShader);
    glLinkProgram(s_shaderProgram);

    glGetProgramiv(s_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar log[512];
        glGetProgramInfoLog(s_shaderProgram, 512, nullptr, log);
        LOG_E("Shader program link failed: %s", log);
    }

    glDeleteShader(vertShader);
    glDeleteShader(geomShader);
    glDeleteShader(fragShader);

    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);

    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(WaterParticle), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, velocity));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, life));

    glBindVertexArray(0);

    LOG("WaterParticleSystem initialized (max: %zu particles)", MAX_PARTICLES);
}

void WaterParticleSystem::Update(float dt) {
    float deltaTime = dt / 1000.f;
    s_activeParticles = 0;

    for (auto& particle : s_particles) {
        if (particle.life > 0.0f) {
            particle.velocity.y -= s_gravity * deltaTime;

            if (particle.velocity.y < -5.f) {
                particle.velocity.y = -5.f;
            }
            particle.position += particle.velocity * deltaTime;
            particle.life -= deltaTime;
            particle.size = 0.04f;

            if (particle.position.y < 0.1f) {
                particle.life = 0.0f;
            }
            s_activeParticles++;
        }
    }

    if (s_activeParticles > 0) {
        UpdateBuffers();
    }
}

void WaterParticleSystem::UpdateBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, s_particles.size() * sizeof(WaterParticle), s_particles.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void WaterParticleSystem::Draw(Shader* mainShader) {
    if (s_activeParticles == 0 || s_shaderProgram == 0) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glUseProgram(s_shaderProgram);

    GLint viewLoc = glGetUniformLocation(mainShader->GetID(), "uView");
    GLint projLoc = glGetUniformLocation(mainShader->GetID(), "uProj");

    GLfloat viewMat[16], projMat[16];
    glGetUniformfv(mainShader->GetID(), viewLoc, viewMat);
    glGetUniformfv(mainShader->GetID(), projLoc, projMat);

    glUniformMatrix4fv(glGetUniformLocation(s_shaderProgram, "uView"), 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(glGetUniformLocation(s_shaderProgram, "uProj"), 1, GL_FALSE, projMat);

    glUniform3f(glGetUniformLocation(s_shaderProgram, "uLightPos"), 10.f, 10.f, 10.f);
    glUniform3f(glGetUniformLocation(s_shaderProgram, "uViewPos"), 0.f, 5.f, 10.f);

    glBindVertexArray(s_vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(s_activeParticles));
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void WaterParticleSystem::Watering(const glm::vec3& centerPos) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> posDist(-0.5f, 0.5f);
    static std::uniform_real_distribution<float> lifeDist(0.8f, 1.2f);
    static std::uniform_real_distribution<float> speedDist(0.f, 0.5f);

    for (auto& particle : s_particles) {
        float offsetX = posDist(gen) * 2.f;
        float offsetZ = posDist(gen) * 2.f;

        particle.position = centerPos + glm::vec3(offsetX, 3.f, offsetZ);
        particle.velocity = glm::vec3(posDist(gen) * 0.2f, -2.f - speedDist(gen), posDist(gen) * 0.2f);
        particle.maxLife = lifeDist(gen);
        particle.life = particle.maxLife;
        particle.size = 0.04f;
    }
}