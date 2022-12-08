#pragma once
#include "InputHeader.h"

namespace VlkEngine {
	class InputSystem {
	public:
		InputSystem(Camera* camera);

		void MouseMovement(GLFWwindow* window, double xpos, double ypos);
		void ProcessInput(GLFWwindow* window,float deltaTime);

	private:
		Camera* mcamera;

		// mouse pos
		float lastXpos;
		float lastYpos;
		bool isInitpos=false;






	};

}