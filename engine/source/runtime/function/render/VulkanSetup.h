#pragma once
#include "RenderHeader.h"

#define  NDEBUG

namespace VlkEngine {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanSetup {
		friend class VulkanEngine;
		friend class RenderPipline;
		friend class RenderBuffer;
		friend class VulkanSyncObject;
		friend class RenderDescriptor;
		friend class RenderImage;
	public:
		VulkanSetup(GLFWwindow* glfwwindow);

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
		// window surface
		VkSurfaceKHR surface;
		GLFWwindow* window;
		VkQueue presentQueue;
		// swap chain
		VkSwapchainKHR swapChain;
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		// image view
		std::vector<VkImageView> swapChainImageViews;

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
		// logical device
		void CreateLogicalDevice();
		void DestroyLogicalDevice();
		// window surface
		void CreateSurface();
		void DestroySurface();
		// swap chain
		void CreateSwapChain();
		void DestroySwapChain();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		// image view 
		void CreateImageViews();
		void DestroyImageViews();
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	public:
		// support format
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
			VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();

	public:
		void InitVulkan();
		void ShutDownVulkan();
		

	};
}