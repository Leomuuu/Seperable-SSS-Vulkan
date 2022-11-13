#include"InputSystem.h"

namespace VlkEngine {


	InputSystem::InputSystem( Camera* camera):
		mcamera(camera)
	{
	}

	void InputSystem::MouseMovement(GLFWwindow* window, double xpos, double ypos)
	{
		if (!isInitpos) {
			int x, y;
			glfwGetWindowSize(window,&x, &y);
			lastXpos = x / 2;
			lastYpos = y / 2;
			isInitpos = true;
		}
		float deltaX = xpos - lastXpos;
		float deltaY = ypos - lastYpos;
		lastXpos = xpos;
		lastYpos = ypos;
		mcamera->ProcessRotation(deltaX, deltaY);
	}

	void InputSystem::ProcessInput(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		if (glfwGetKey(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		float movex = 0.0f, movey = 0.0f, movez = 0.0f;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			movex += 0.01f;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			movex -= 0.01f;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			movey -= 0.01f;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			movey += 0.01f;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			movez += 0.01f;
		}
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
			movez -= 0.01f;
		}
		mcamera->ProcessMovement(movex, movey, movez);
	}
}