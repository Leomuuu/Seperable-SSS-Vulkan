#pragma once

#include "RenderHeader.h"
#include "RenderBuffer.h"
#include "../platform/FileService.h"

namespace VlkEngine {
	class RenderImage {
		friend class VulkanEngine;
		friend class RenderDescriptor;
	public:
		RenderImage(RenderBuffer* renderbuffer);

	private:
		RenderBuffer* renderBuffer;

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

		void CreateImage(uint32_t width, uint32_t height,
			VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
			VkImage& image, VkDeviceMemory& imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format,
			VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	};

}
