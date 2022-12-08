#pragma once
#include "RenderHeader.h"
#include "../platform/FileService.h"


namespace VlkEngine {
	class VulkanEngine;

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

	class VulkanBase {

		friend class VulkanEngine;
		friend class Editor;
	protected:
		// window
		GLFWwindow* window;
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
		QueueFamilyIndices indices;
		VkQueue graphicsQueue;
		// window surface
		VkSurfaceKHR surface;
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
		// render pass 
		VkRenderPass renderPass;
		// render pipeline
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		// Descriptor
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		// frame buffer
		std::vector<VkFramebuffer> swapChainFramebuffers;
		// command pool command buffer
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;
		// vertex buffer
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		// index buffer
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		// vertex shader stage uniform buffer
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		// std::vector<void*> uniformBuffersMapped;
		// fragment shader stage uniform buffer
		std::vector<VkBuffer> fraguniformBuffers;
		std::vector<VkDeviceMemory> fraguniformBuffersMemory;
		std::vector<void*> fraguniformBuffersMapped;
		// TextureImage
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
		// DepthBuffer
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;
		// SyncObject
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

	protected:
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
		// Render Pass
		void CreateRenderPass();
		void DestroyRenderPass();
		// Graphics Pipeline
		virtual void CreateGraphicsPipeline();
		void DestroyGraphicsPipeline();
		// Descriptor
		virtual void CreateDescriptorSetLayout();
		virtual void CreateDescriptorPool();
		virtual void CreateDescriptorSets();
		void DestroyDescriptor();
		// frame buffer
		void CreateFramebuffers();
		void DestroyFramebuffers();
		// command pool command buffer
		void CreateCommandPool();
		void DestroyCommandPool();
		void CreateCommandBuffer();
		// vertex buffer
		void CreateVertexBuffer();
		void DestroyVertexBuffer();
		// index buffer
		void CreateIndexBuffer();
		void DestroyIndexBuffer();
		// uniform buffer
		void CreateUniformBuffers();
		void DestroyUniformBuffers();
		// dynamic uniform buffer
		std::vector<void*> dynamicUniformData;
		size_t dynamicAlignment;
		size_t normalUBOAlignment;
		DynamicUBO uboDynamic;
		// TextureImage
		virtual void CreateTexture();
		virtual void DestroyTexture();
		// DepthBuffer
		void CreateDepthResource();
		void DestroyDepthResource();
		// SyncObject
		void CreateSyncObjects();
		void DestroySyncObjects();

		


	protected:
		// support format
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
			VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		// shader module
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		// buffer
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		VkCommandBuffer PreSingleTimeCommands(VkCommandPool cmdPool);
		void PostSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool cmdPool);
		// image
		bool HasStencilComponent(VkFormat format);
		void CreateImage(uint32_t width, uint32_t height,
			VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
			VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format,
			VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	public:
		VulkanEngine* engine;

	public:
		VulkanBase(GLFWwindow* glfwwindow,VulkanEngine* vlkengine);

		void StartVulkan();
		void ShutDownVulkan();

	};

}