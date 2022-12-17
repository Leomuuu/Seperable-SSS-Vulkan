#pragma once
#include "RenderHeader.h"
#include "../platform/FileService.h"


namespace VlkEngine {

	static void* alignedAlloc(size_t size, size_t alignment)
	{
		void* data = nullptr;
	#if defined(_MSC_VER) || defined(__MINGW32__)
		data = _aligned_malloc(size, alignment);
	#else
		int res = posix_memalign(&data, alignment, size);
		if (res != 0)
			data = nullptr;
	#endif
		return data;
	}


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
		// dynamic uniform buffer
		std::vector<void*> dynamicUniformData;
		size_t dynamicAlignment;
		size_t normalUBOAlignment;
		DynamicUBO uboDynamic;
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
		virtual void CreateRenderPass();
		virtual void DestroyRenderPass();
		// Graphics Pipeline
		virtual void CreateGraphicsPipeline();
		virtual void DestroyGraphicsPipeline();
		// Descriptor
		virtual void CreateDescriptorSetLayout();
		virtual void CreateDescriptorPool();
		virtual void CreateDescriptorSets();
		virtual void DestroyDescriptor();
		// frame buffer
		virtual void CreateFramebuffers();
		virtual void DestroyFramebuffers();
		// command pool command buffer
		virtual void CreateCommandPool();
		virtual void DestroyCommandPool();
		virtual void CreateCommandBuffer();
		// vertex buffer
		virtual void CreateVertexBuffer();
		virtual void DestroyVertexBuffer();
		// index buffer
		virtual void CreateIndexBuffer();
		virtual void DestroyIndexBuffer();
		// uniform buffer
		virtual void CreateUniformBuffers();
		virtual void DestroyUniformBuffers();
		// TextureImage
		virtual void CreateTexture();
		virtual void DestroyTexture();
		// DepthBuffer
		virtual void CreateDepthResource();
		virtual void DestroyDepthResource();
		// SyncObject
		virtual void CreateSyncObjects();
		virtual void DestroySyncObjects();

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

		void CreateVulkanResources();
		void DestroyVulkanResources();

		virtual void StartVulkan();
		virtual void ShutDownVulkan();

		virtual void RecordCommandBuffer(uint32_t imageIndex, uint32_t currentFrame);
		virtual void UpdateUniformBuffer(uint32_t currentImage);

	protected:
		std::string lightVertPath;
		std::string lightFragPath;

	};

}