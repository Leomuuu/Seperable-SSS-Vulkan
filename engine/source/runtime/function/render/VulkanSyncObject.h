#include "RenderHeader.h"

namespace VlkEngine {
	class VulkanSyncObject {
		friend class VulkanEngine;

	public:
		VulkanSyncObject(VkDevice& vkdevice);
	private:
		VkDevice& device;


	public:
		void CreateSyncObjects();
		void DestroySyncObjects();

	private:
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkFence inFlightFence;



	};

}