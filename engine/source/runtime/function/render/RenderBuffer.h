#pragma once

#include "VulkanSetup.h"
#include "RenderPipline.h"

namespace VlkEngine {



	class RenderBuffer {
		friend class VulkanEngine;
		public:
			RenderBuffer(VulkanSetup* vulkansetup,RenderPipline* renderpipline);
		private:
			VulkanSetup* vulkanSetup;
			RenderPipline* renderPipline;

		public:
			void CreateBuffers();
			void DestroyBuffers();

			
		private:
			// frame buffer
			void CreateFramebuffers();
			void DestroyFramebuffers();
			// command pool
			void CreateCommandPool();
			void DestroyCommandPool();
			// command buffer
			void CreateCommandBuffer();
			void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
			// vertex buffer
			void CreateVertexBuffer();
			void DestroyVertexBuffer();
			uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
			void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
				VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
			void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
			// index buffer
			void CreateIndexBuffer();
			void DestroyIndexBuffer();

		private:
			// frame buffer
			std::vector<VkFramebuffer> swapChainFramebuffers;
			// command pool
			VkCommandPool commandPool;
			// command buffer
			std::vector<VkCommandBuffer> commandBuffers;
			// vertex buffer
			VkBuffer vertexBuffer;
			VkDeviceMemory vertexBufferMemory;
			// index buffer
			VkBuffer indexBuffer;
			VkDeviceMemory indexBufferMemory;

	};

}