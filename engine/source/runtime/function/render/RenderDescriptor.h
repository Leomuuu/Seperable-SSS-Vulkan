#pragma once

#include "RenderHeader.h"
#include "VulkanSetup.h"
#include "RenderBuffer.h"
#include "RenderImage.h"
#include "../../VulkanEngine.h"

namespace VlkEngine {

	class RenderDescriptor {
		friend class RenderPipline;
		friend class RenderBuffer;
		friend class VulkanEngine;
		friend class Editor;
	public:
		RenderDescriptor(VulkanEngine* vlkengine);
		void DestroyDescriptor();

		// DescriptorSetLayout
		void CreateDescriptorSetLayout();
		// DescriptorPool
		void CreateDescriptorPool();
		// DescriptorSet
		void CreateDescriptorSets(RenderBuffer* renderBuffer, RenderImage* renderImage);
	
	private:
		VulkanEngine* engine;

		// DescriptorSetLayout
		VkDescriptorSetLayout descriptorSetLayout;
		// DescriptorPool
		VkDescriptorPool descriptorPool;
		// DescriptorSet
		std::vector<VkDescriptorSet> descriptorSets;
	};

}