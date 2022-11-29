#include "../runtime/VulkanEngine.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace VlkEngine {
	class Editor {
	private:
		VulkanEngine* renderEngine;

	public:
		Editor(VulkanEngine* engine);
		void Run();

	private:
		void StartEditor();
		void MainLoop();
		void ShutDownEditor();

	private:
		void InitUI();
		void createUIDescriptorPool();
		void createUIRenderPass();
		void createUICommandPool(VkCommandPool* cmdPool, VkCommandPoolCreateFlags flags);
		void createUICommandBuffers();
		void createUIFramebuffers();
		void DrawUI();
		void recordUICommands(uint32_t bufferIndex);
		void DrawFrame();

	private:
		std::vector<VkFramebuffer> uiFramebuffers;
		VkRenderPass uiRenderPass;
		VkCommandPool uiCommandPool;
		std::vector<VkCommandBuffer> uiCommandBuffers;
		VkDescriptorPool uiDescriptorPool;
	};

}