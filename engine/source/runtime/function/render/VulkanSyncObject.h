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
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;



	};

}