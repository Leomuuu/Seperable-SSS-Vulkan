#include "../runtime/VulkanEngine.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <time.h>

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
		void CreateUIDescriptorPool();
		void CreateUIRenderPass();
		void CreateUICommandPool(VkCommandPool* cmdPool, VkCommandPoolCreateFlags flags);
		void CreateUICommandBuffers();
		void CreateUIFramebuffers();
		void DestroyUIFramebuffers();
		void RecordUICommandBuffer(uint32_t bufferIndex);
		void WindowSurfaceChange();

		void DrawUI();
		void DrawFrame();

		float Calfps(float DeltaTime);

	private:
		std::vector<VkFramebuffer> uiFramebuffers;
		VkRenderPass uiRenderPass;
		VkCommandPool uiCommandPool;
		std::vector<VkCommandBuffer> uiCommandBuffers;
		VkDescriptorPool uiDescriptorPool;

		float fps=0;
	};

}