#include "RenderBuffer.h"

namespace VlkEngine {
	RenderBuffer::RenderBuffer(VulkanEngine* vlkengine):
		engine(vlkengine)
	{

	}
	void RenderBuffer::CreateBuffers()
	{
		CreateFramebuffers();
		CreateCommandPool();
		CreateCommandBuffer();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateUniformBuffers();
	}

	void RenderBuffer::DestroyBuffers()
	{
		DestroyUniformBuffers();
		DestroyIndexBuffer();
		DestroyVertexBuffer();
		DestroyCommandPool();
		DestroyFramebuffers();
	}

	void RenderBuffer::CreateFramebuffers()
	{
		swapChainFramebuffers.resize(engine->vulkanSetup->swapChainImageViews.size());
		for (size_t i = 0; i < engine->vulkanSetup->swapChainImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				engine->vulkanSetup->swapChainImageViews[i],
				engine->renderImage->depthImageView
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = engine->renderPipline->renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = engine->vulkanSetup->swapChainExtent.width;
			framebufferInfo.height = engine->vulkanSetup->swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(engine->vulkanSetup->device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

	}

	void RenderBuffer::DestroyFramebuffers()
	{
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(engine->vulkanSetup->device, framebuffer, nullptr);
		}
	}

	void RenderBuffer::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = engine->vulkanSetup->FindQueueFamilies(engine->vulkanSetup->physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(engine->vulkanSetup->device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void RenderBuffer::DestroyCommandPool()
	{
		vkDestroyCommandPool(engine->vulkanSetup->device, commandPool, nullptr);
	}

	void RenderBuffer::CreateCommandBuffer()
	{
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(engine->vulkanSetup->device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void RenderBuffer::CreateVertexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(engine->modelManager->vertices[0]) * engine->modelManager->vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(engine->vulkanSetup->device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, engine->modelManager->vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(engine->vulkanSetup->device, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
	
		CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(engine->vulkanSetup->device, stagingBuffer, nullptr);
		vkFreeMemory(engine->vulkanSetup->device, stagingBufferMemory, nullptr);
	}

	void RenderBuffer::DestroyVertexBuffer()
	{
		vkDestroyBuffer(engine->vulkanSetup->device, vertexBuffer, nullptr);
		vkFreeMemory(engine->vulkanSetup->device, vertexBufferMemory, nullptr);
	}

	uint32_t RenderBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(engine->vulkanSetup->physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
		return 0;
	}

	void RenderBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(engine->vulkanSetup->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(engine->vulkanSetup->device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(engine->vulkanSetup->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(engine->vulkanSetup->device, buffer, bufferMemory, 0);
	}

	void RenderBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = PreSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		PostSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer RenderBuffer::PreSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(engine->vulkanSetup->device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void RenderBuffer::PostSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(engine->vulkanSetup->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(engine->vulkanSetup->graphicsQueue);

		vkFreeCommandBuffers(engine->vulkanSetup->device, commandPool, 1, &commandBuffer);
	}

	void RenderBuffer::CreateIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(engine->modelManager->indices[0]) * engine->modelManager->indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(engine->vulkanSetup->device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, engine->modelManager->indices.data(), (size_t)bufferSize);
		vkUnmapMemory(engine->vulkanSetup->device, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(engine->vulkanSetup->device, stagingBuffer, nullptr);
		vkFreeMemory(engine->vulkanSetup->device, stagingBufferMemory, nullptr);
	}

	void RenderBuffer::DestroyIndexBuffer()
	{
		vkDestroyBuffer(engine->vulkanSetup->device, indexBuffer, nullptr);
		vkFreeMemory(engine->vulkanSetup->device, indexBufferMemory, nullptr);
	}

	void RenderBuffer::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(engine->vulkanSetup->device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}
	}

	void RenderBuffer::DestroyUniformBuffers()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(engine->vulkanSetup->device, uniformBuffers[i], nullptr);
			vkFreeMemory(engine->vulkanSetup->device, uniformBuffersMemory[i], nullptr);
		}
	}
}