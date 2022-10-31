#include "RenderHeader.h"

namespace VlkEngine {

	class RenderPipline {
	public:
		RenderPipline(VkDevice& vkdevice);

	public:
		// Render Pass
		void CreateRenderPass(VkFormat& swapChainImageFormat);
		void DestroyRenderPass();
		// Graphics Pipeline
		void CreateGraphicsPipeline(std::string& vertShaderPath,std::string& fragShaderPath);
		void DestroyGraphicsPipeline();
		

	private:
		VkDevice& device;
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

	private:
		// shader module
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

	};

}