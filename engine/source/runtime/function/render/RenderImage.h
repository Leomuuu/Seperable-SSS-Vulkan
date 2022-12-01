#pragma once

#include "RenderHeader.h"
#include "RenderBuffer.h"
#include "../platform/FileService.h"

namespace VlkEngine {
	class RenderImage {
		friend class VulkanEngine;
		friend class RenderDescriptor;
		friend class RenderBuffer;
	public:
		RenderImage(VulkanEngine* vlkengine);

	private:
		VulkanEngine* engine;

		// TextureImage
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		void CreateTextureImage();
		void DestroyTextureImage();
		// TextureImageView
		VkImageView textureImageView;
		void CreateTextureImageView();
		void DestroyTextureImageView();
		// TextureSampler
		VkSampler textureSampler;
		void CreateTextureSampler();
		void DestroyTextureSampler();

		// DepthBuffer
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;
		void CreateDepthResource();
		void DestroyDepthResource();
		bool HasStencilComponent(VkFormat format);


		void CreateImage(uint32_t width, uint32_t height,
			VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
			VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format,
			VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	};

}
