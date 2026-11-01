#include "PCH.h"
#include "Camera.h"

Camera::Camera()
{
	Init();
}

void Camera::Init()
{
	yaw = 0.f;
	pitch = 20.f;
	distance = 10.f;

	eye = { 0.0f, 3.0f, -distance };
	at = { 0.0f, 0.0f, 0.0f };
	up = { 0.0f, 1.0f, 0.0f };
}

glm::vec3 Camera::GetForward() const
{
	return glm::normalize(at - eye);
}

glm::vec3 Camera::GetForwardFlat() const
{
	glm::vec3 f = GetForward();
	f.y = 0.f;
	return glm::normalize(f);
}

glm::vec3 Camera::GetRightFlat() const
{
	return glm::normalize(glm::cross(GetForwardFlat(), glm::vec3(0, 1, 0)));
}

float Camera::GetFlatYaw() const
{
	glm::vec3 f = GetForwardFlat();
	return glm::degrees(std::atan2(f.x, f.z));
}

void Camera::AddYaw(float delta)
{
	const float MAX_DELTA = 4.0f;

	delta = glm::clamp(delta, -MAX_DELTA, MAX_DELTA);

	yaw += delta;
	yaw = glm::mod(yaw, 360.0f);
	if (yaw < 0.f) yaw += 360.0f;
}

void Camera::AddPitch(float delta)
{
	pitch += delta;
	pitch = glm::clamp(pitch, 10.f, 50.f);
}

void Camera::FollowTarget(const glm::vec3& targetPos)
{
	at = targetPos + glm::vec3(0.f, 1.5f, 0.f);

    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);

    float h = cos(pitchRad) * distance;
    float v = sin(pitchRad) * distance;

	eye.x = at.x - h * sin(yawRad);
	eye.y = at.y + v;
	eye.z = at.z + h * cos(yawRad);
}

glm::vec2 Camera::WorldToScreen(const glm::vec3& worldPos, float screenW, float screenH) const
{
	glm::mat4 view = GetView();
	glm::mat4 proj = GetProj(screenW / screenH);

	glm::vec4 clip = proj * view * glm::vec4(worldPos, 1.0f);

	if (clip.w <= 0)
		return glm::vec2(-9999, -9999); // 화면 뒤에 있음 = 안 보이게

	glm::vec3 ndc = glm::vec3(clip) / clip.w;

	float x = (ndc.x * 0.5f + 0.5f) * screenW;
	float y = (ndc.y * 0.5f + 0.5f) * screenH;

	return { x, y };
}



glm::mat4 Camera::GetView() const
{
	return glm::lookAt(eye, at, up);
}

glm::mat4 Camera::GetProj(float aspect) const
{
	return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.f);
}