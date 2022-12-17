#pragma once
#include "VulkanBase.h"

namespace VlkEngine {
	class VulkanEngine;

	class VulkanPBR:public  VulkanBase {
	protected:
		// PBR Texture
		VkImage pbrTextureImage[6];
		VkDeviceMemory pbrTextureImageMemory[6];
		VkImageView pbrTextureImageView[6];
		VkSampler pbrTextureSampler[6];

	protected:
		// TextureImage
		virtual void CreateTexture();
		virtual void DestroyTexture();

		// Descriptor
		virtual void CreateDescriptorSetLayout();
		virtual void CreateDescriptorPool();
		virtual void CreateDescriptorSets();

		// Graphics Pipeline
		virtual void CreateGraphicsPipeline();

	public:
		VulkanPBR(GLFWwindow* glfwwindow, VulkanEngine* vlkengine);

		virtual void StartVulkan();
		virtual void ShutDownVulkan();
	};
}