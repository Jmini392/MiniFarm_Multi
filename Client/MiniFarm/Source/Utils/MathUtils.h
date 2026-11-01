#pragma once

inline glm::mat4 Translate(const glm::vec3& t)
{
	return glm::translate(glm::mat4(1.0f), t);
}

inline glm::mat4 Rotate(float deg, const glm::vec3& axis)
{
	return glm::rotate(glm::mat4(1.0f), glm::radians(deg), axis);
}

inline glm::mat4 Scale(const glm::vec3& s)
{
	return glm::scale(glm::mat4(1.0f), s);
}

inline glm::mat4 PivotRotate(float deg, const glm::vec3& axis, const glm::vec3& pivot)
{
	return glm::translate(glm::mat4(1), pivot)
		* glm::rotate(glm::mat4(1), glm::radians(deg), axis)
		* glm::translate(glm::mat4(1), -pivot);
}

inline glm::mat4 PivotScale(float s, const glm::vec3& pivot)
{
	return glm::translate(glm::mat4(1), pivot)
		* glm::scale(glm::mat4(1), glm::vec3(s))
		* glm::translate(glm::mat4(1), -pivot);
}