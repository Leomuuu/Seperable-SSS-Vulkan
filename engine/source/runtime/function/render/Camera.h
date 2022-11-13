#pragma once
#include "RenderHeader.h"

namespace VlkEngine {
	class Camera {
		friend class VulkanEngine;
	public:
		Camera(glm::vec3 position, glm::vec3 target, glm::vec3 worldup,
			float fov, float znear,float zfar);

		glm::mat4 Camera::GetViewMatrix();
		void ProcessRotation(float deltaX, float deltaY);
		void ProcessMovement(float deltaX, float deltaY, float deltaZ);
		void ProcessScale(float delta);

	private:
		glm::vec3 camPosition;
		glm::vec3 camForward;
		glm::vec3 camRight;
		glm::vec3 camUp;
		glm::vec3 worldUp;
		float mousespeedX = 0.1f;
		float mousespeedY = 0.1f;
		float movespeedX = 0.01f;
		float movespeedY = 0.01f;
		float movespeedZ = 0.01f;
		float scalespeed = 0.01f;
		float Fov;
		float zNear;
		float zFar;
	};

}