#pragma once
#include "VulkanShadowMap.h"

namespace VlkEngine {
	class VulkanSSSS :public VulkanShadowMap {

	protected:
		// offscreen light rendering
		struct OffscreenPass {
			int32_t width, height;
			VkFramebuffer frameBuffer;
			VkRenderPass renderPass;
			VkImage image;
			VkDeviceMemory deviceMemory;
			VkImageView imageView;
			VkSampler sampler;
		} offscreenLightPass;

		struct OffscreenDescriptor {
			VkDescriptorSetLayout descriptorSetLayout;
			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSets;
		}offscreenLightDescriptor;

		struct OffscreenUniformBuffer {
			// vertex stage uniform buffer
			std::vector<VkBuffer> uniformBuffers;
			std::vector<VkDeviceMemory> uniformBuffersMemory;
			std::vector<void*> dynamicUniformData;
			size_t dynamicAlignment;
			size_t normalUBOAlignment;
			DynamicUBO uboDynamic;
			// fragment stage uniform buffer
			std::vector<VkBuffer> fraguniformBuffers;
			std::vector<VkDeviceMemory> fraguniformBuffersMemory;
			std::vector<void*> fraguniformBuffersMapped;
		}offscreenLightUniformBuffer;

		struct OffscreenPipeline {
			VkPipeline pipeline;
			VkPipelineLayout pipelineLayout;
		}offscreenLightPipeline;

		struct OffscreenDepthResource {
			VkImage depthImage;
			VkDeviceMemory depthImageMemory;
			VkImageView depthImageView;
		} offscreenLightDepthResource;

	protected:
		void CreateOffscreenLightRenderpass();
		void CreateOffscreenLightImage();
		void CreateOffscreenLightFrameBuffer();
		void CreateOffscreenLightUniformBuffers();
		void CreateOffscreenLightDescriptorSetLayout();
		void CreateOffscreenLightDescriptorSets();
		void CreateOffscreenLightPipeline();
		void CreateOffscreenLightDepthResource();
		void DestroyOffscreenLightResources();


	public:
		VulkanSSSS(GLFWwindow* glfwwindow, VulkanEngine* vlkengine);

		virtual void StartVulkan();
		virtual void ShutDownVulkan();

		virtual void RecordCommandBuffer(uint32_t imageIndex, uint32_t currentFrame);
		virtual void UpdateUniformBuffer(uint32_t currentImage);

	protected:
		// Main Pass
		// Descriptor
		virtual void CreateDescriptorSetLayout();
		virtual void CreateDescriptorPool();
		virtual void CreateDescriptorSets();
		// Graphics Pipeline
		virtual void CreateGraphicsPipeline();


	};
}