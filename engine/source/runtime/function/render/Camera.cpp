#include "Camera.h"

namespace VlkEngine {


	Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 worldup,
		float fov, float znear, float zfar):
		camPosition(position),worldUp(glm::normalize(worldup)),Fov(fov),zNear(znear),zFar(zfar)
	{
		camForward = glm::normalize(target - position);

		camRight = glm::normalize(glm::cross(camForward, worldUp));
		camUp = glm::normalize(glm::cross(camRight, camForward));
	}

	void Camera::ProcessRotation(float deltaX, float deltaY)
	{

		float pitch = deltaY * mousespeedY;
		float yaw = deltaX * mousespeedX;
		
		glm::quat q1 = glm::angleAxis(glm::radians(pitch), camRight);
		glm::quat q2 = glm::angleAxis(glm::radians(yaw), camUp);
		glm::quat q = q1 * q2;
		
		camForward = glm::normalize(q * camForward);

		camRight = glm::normalize(glm::cross(camForward, camUp));
		camUp = glm::normalize(glm::cross(camRight, camForward));
	}


	void Camera::ProcessScale(float delta)
	{
		Fov += scalespeed*delta;
		if (Fov < 1.0f) Fov = 1.0f;
		if (Fov > 45.0f) Fov = 45.0f;
	}

	void Camera::ProcessMovement(float deltaX, float deltaY, float deltaZ)
	{
		camPosition += camForward * movespeedX * deltaX;
		camPosition += camRight * movespeedX * deltaY;
		camPosition += camUp * movespeedX * deltaZ;
	}


	glm::mat4 Camera::GetViewMatrix()
	{
		return glm::lookAt(camPosition, camPosition + camForward, worldUp);
	}
}