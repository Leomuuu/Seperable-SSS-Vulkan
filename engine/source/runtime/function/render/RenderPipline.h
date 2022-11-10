#pragma  once
#include "RenderHeader.h"
#include "VulkanSetup.h"
#include "RenderDescriptor.h"

namespace VlkEngine {

	class RenderPipline {
		friend class RenderBuffer;
		friend class VulkanEngine;
	public:
		RenderPipline(VulkanSetup* vulkansetup, RenderDescriptor* renderdescriptor);

	public:
		// Render Pass
		void CreateRenderPass(VkFormat& swapChainImageFormat);
		void DestroyRenderPass();
		// Graphics Pipeline
		void CreateGraphicsPipeline(std::string& vertShaderPath,std::string& fragShaderPath);
		void DestroyGraphicsPipeline();
		

	private:
		VulkanSetup* vulkanSetup;
		RenderDescriptor* renderDescriptor;

		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

	private:
		// shader module
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

	};

}