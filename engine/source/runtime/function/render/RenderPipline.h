#pragma once

#include "RenderHeader.h"
#include "VulkanSetup.h"
#include "RenderDescriptor.h"
#include "../platform/FileService.h"

namespace VlkEngine {

	class RenderPipline {
		friend class RenderBuffer;
		friend class VulkanEngine;
	public:
		RenderPipline(VulkanEngine* vlkengine);

	public:
		// Render Pass
		void CreateRenderPass(VkFormat& swapChainImageFormat);
		void DestroyRenderPass();
		// Graphics Pipeline
		void CreateGraphicsPipeline(std::string& vertShaderPath,std::string& fragShaderPath);
		void DestroyGraphicsPipeline();
		

	private:
		VulkanEngine* engine;

		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

	private:
		// shader module
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

	};

}