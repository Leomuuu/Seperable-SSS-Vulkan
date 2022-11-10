#include "VulkanSyncObject.h"

namespace VlkEngine {
	

	VulkanSyncObject::VulkanSyncObject(VulkanSetup* vulkansetup):
		vulkanSetup(vulkansetup)
	{

	}

	void VulkanSyncObject::CreateSyncObjects()
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(vulkanSetup->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(vulkanSetup->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(vulkanSetup->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void VulkanSyncObject::DestroySyncObjects()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(vulkanSetup->device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(vulkanSetup->device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(vulkanSetup->device, inFlightFences[i], nullptr);
		}
	}
}