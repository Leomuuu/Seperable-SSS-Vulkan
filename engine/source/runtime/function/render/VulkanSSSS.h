#pragma once
#include "VulkanShadowMap.h"

#define SSSS_SAMPLES 25 
#define SSSS_WIDTH 0.015f
#define DTRANSLUCENCY -0.05
namespace VlkEngine {
	class VulkanSSSS :public VulkanShadowMap {

	protected:
	/************  offscreen light rendering pass begin ************/ 
		struct OffscreenPass {
			int32_t width, height;
			VkFramebuffer frameBuffer;
			VkRenderPass renderPass;
			VkImage image[2]; // diffuse , specular
			VkDeviceMemory deviceMemory[2];
			VkImageView imageView[2];
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
		} offscreenDepthResource;

		void CreateOffscreenLightImage();
		void CreateOffscreenDepthResource();
		void CreateOffscreenLightRenderpass();
		void CreateOffscreenLightFrameBuffer();
		void CreateOffscreenLightUniformBuffers();
		void CreateOffscreenLightDescriptorSetLayout();
		void CreateOffscreenLightPipeline();
		void CreateOffscreenLightDescriptorSets();
		void DestroyOffscreenLightResources();
	/************ offscreen light rendering pass end ************/


	/************  offscreen sssblur pass begin ************/
		struct SSSBlurPass {
			VkImage image[2];  
			VkDeviceMemory deviceMemory[2];
			VkImageView imageView[2];
			VkFramebuffer frameBuffer[2];
			VkRenderPass renderPass[2];
		} sssBlurPass;

		struct SSSBlurDescriptor {
			VkDescriptorSetLayout descriptorSetLayout;
			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSets;
		}sssBlurDescriptor;

		struct SSSBlurPipeline {
			VkPipeline pipeline[2];
			VkPipelineLayout pipelineLayout;
		}sssBlurPipeline;

		void CreateSSSBlurImage();
		void CreareSSSBlurRenderPass();
		void CreareSSSBlurFrameBuffer();
		void CreateSSSBlurDescriptorSetLayout();
		void CreateSSSBlurPipeline();
		void CreateSSSBlurDescriptorSets();


	/************  offscreen sssblur pass end ************/


	public:
		VulkanSSSS(GLFWwindow* glfwwindow, VulkanEngine* vlkengine);

		virtual void StartVulkan();
		virtual void ShutDownVulkan();
		virtual void RecordCommandBuffer(uint32_t imageIndex, uint32_t currentFrame);
		virtual void UpdateUniformBuffer(uint32_t currentImage);



	protected:
		/************ main pass begin ************/

		// Descriptor
		virtual void CreateDescriptorSetLayout();
		virtual void CreateDescriptorPool();
		virtual void CreateDescriptorSets();
		// Graphics Pipeline
		virtual void CreateGraphicsPipeline();

		/************ main pass end *************/


	};
}