#pragma once
#include "VulkanPBR.h"

#define SHADOWMAP_DIMENSION 2048
#define DEPTH_FORMAT VK_FORMAT_D16_UNORM


namespace VlkEngine {
	class VulkanShadowMap :public VulkanPBR {

	protected:
		// offscreen shadow rendering
		struct OffscreenPass {
			int32_t width, height;
			VkFramebuffer frameBuffer;
			VkRenderPass renderPass;
			VkImage image;
			VkDeviceMemory deviceMemory;
			VkImageView imageView;
			VkSampler sampler;
			VkDescriptorImageInfo descriptor;
		} offscreenShadowPass;

		struct OffscreenDescriptor {
			VkDescriptorSetLayout descriptorSetLayout;
			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSets;
		}offscreenShadowDescriptor;

		struct OffscreenUniformBuffer {
			std::vector<VkBuffer> uniformBuffers;
			std::vector<VkDeviceMemory> uniformBuffersMemory;
			std::vector<void*> dynamicUniformData;
			size_t dynamicAlignment;
			size_t normalUBOAlignment;
			DynamicUBO uboDynamic;
		}offscreenShadowUniformBuffer;

		struct OffscreenPipeline {
			VkPipeline Pipeline;
			VkPipelineLayout pipelineLayout;
		}offscreenShadowPipeline;
		

		float shadowMapZNear = 0.1f;
		float shadowMapZFar = 10.0f;
		// Depth bias (and slope) are used to avoid shadowing artifacts
		// Constant depth bias factor (always applied)
		float depthBiasConstant = 1.25f;
		// Slope depth bias factor, applied depending on polygon's slope
		float depthBiasSlope = 1.75f;
		


	protected:
		void CreateOffscreenShadowRenderpass();
		void CreateOffscreenShadowImage();
		void CreateOffscreenShadowFrameBuffer();
		void CreateOffscreenShadowUniformBuffers();
		void CreateOffscreenShadowDescriptorSetLayout();
		void CreateOffscreenShadowDescriptorSets();
		void CreateOffscreenShadowPipeline();
		void DestroyOffscreenShadowResources();
		

	public:
		VulkanShadowMap(GLFWwindow* glfwwindow, VulkanEngine* vlkengine);
		
		virtual void StartVulkan();
		virtual void ShutDownVulkan();
		
		virtual void RecordCommandBuffer(uint32_t imageIndex, uint32_t currentFrame);
		virtual void UpdateUniformBuffer(uint32_t currentImage);

	protected:
		// Descriptor
		virtual void CreateDescriptorSetLayout();
		virtual void CreateDescriptorPool();
		virtual void CreateDescriptorSets();

		// Graphics Pipeline
		virtual void CreateGraphicsPipeline();
	};
}