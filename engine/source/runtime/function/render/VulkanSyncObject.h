#include "RenderHeader.h"
#include "VulkanSetup.h"

namespace VlkEngine {
	class VulkanSyncObject {
		friend class VulkanEngine;

	public:
		VulkanSyncObject(VulkanSetup* vulkansetup);
	private:
		VulkanSetup* vulkanSetup;


	public:
		void CreateSyncObjects();
		void DestroySyncObjects();

	private:
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;



	};

}