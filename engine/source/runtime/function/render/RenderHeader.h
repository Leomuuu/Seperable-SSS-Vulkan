#pragma once
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define NOMINMAX

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <optional>
#include <string>

namespace VlkEngine {
	const int MAX_FRAMES_IN_FLIGHT = 2;

}
