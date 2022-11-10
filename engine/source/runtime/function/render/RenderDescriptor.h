#pragma once

#include "RenderHeader.h"
#include "VulkanSetup.h"
#include "RenderBuffer.h"

namespace VlkEngine {
	class RenderDescriptor {
		friend class RenderPipline;
		friend class RenderBuffer;
		friend class VulkanEngine;
	public:
		RenderDescriptor(VulkanSetup* vulkansetup);
		void DestroyDescriptor();

		// DescriptorSetLayout
		void CreateDescriptorSetLayout();
		// DescriptorPool
		void CreateDescriptorPool();
		// DescriptorSet
		void CreateDescriptorSets(RenderBuffer* renderBuffer);
	
	private:
		VulkanSetup* vulkanSetup;

		// DescriptorSetLayout
		VkDescriptorSetLayout descriptorSetLayout;
		// DescriptorPool
		VkDescriptorPool descriptorPool;
		// DescriptorSet
		std::vector<VkDescriptorSet> descriptorSets;
	};

}