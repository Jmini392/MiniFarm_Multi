#pragma once

class Camera {
public:
    Camera();

    void Init();

    glm::mat4 GetView() const;
    glm::mat4 GetProj(float aspect) const;

    glm::vec3 GetForward() const;
    glm::vec3 GetForwardFlat() const;
    glm::vec3 GetRightFlat() const;
    float GetFlatYaw() const;

    // 카메라 회전
    void AddYaw(float delta);
    void AddPitch(float delta);

    void FollowTarget(const glm::vec3& targetPos);
    glm::vec2 WorldToScreen(const glm::vec3& worldPos, float screenW, float screenH) const;
public:
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;

    float yaw;         // 좌우 회전
    float pitch;       // 위/아래 회전
    float distance;    // at(플레이어) 로부터 거리
};
