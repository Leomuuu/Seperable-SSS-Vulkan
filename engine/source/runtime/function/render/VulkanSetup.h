#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <optional>

#define  NDEBUG

namespace VlkEngine {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;

		bool IsComplete() {
			return graphicsFamily.has_value();
		}
	};

	class VulkanSetup {
	private:
		// vulkan instance
		VkInstance instance;
		// validation layer
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		VkDebugUtilsMessengerEXT debugMessenger;
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
		// physical device
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		// logical device
		VkDevice device;
		// queue handle
		VkQueue graphicsQueue;

	private:
		// vulkan instance
		void CreateVulkanInstance();
		void DestroyVulkanInstance();
		// validation layer
		void SetupDebugMessenger();
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, 
			VkDebugUtilsMessengerEXT* pDebugMessenger);
		void DestroyDebugUtilsMessengerEXT(VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator);
		// physical device
		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		int RateDeviceSuitability(VkPhysicalDevice device);
		// queue family
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		//logical device
		void CreateLogicalDevice();
		void DestroyLogicalDevice();


	public:
		void InitVulkan();
		void ShutDownVulkan();
		

	};
}