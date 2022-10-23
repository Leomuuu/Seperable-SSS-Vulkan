#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>


namespace VlkEngine {
	class RenderSystem {
	private:
		VkInstance instance;

	public:
		bool CreateInstance();
		void DestroyInstance();

	};
}