#include "VulkanSSSS.h"
#include "../../VulkanEngine.h"

namespace VlkEngine {

	void VulkanSSSS::CreateOffscreenLightRenderpass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &offscreenLightPass.renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanSSSS::CreateOffscreenLightImage()
	{
		offscreenLightPass.width = swapChainExtent.width;
		offscreenLightPass.height = swapChainExtent.height;

		CreateImage(offscreenLightPass.width, offscreenLightPass.height,
			swapChainImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, offscreenLightPass.image, offscreenLightPass.deviceMemory);
		offscreenLightPass.imageView = CreateImageView(offscreenLightPass.image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (vkCreateSampler(device, &samplerInfo, nullptr, &offscreenLightPass.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create offscreen light sampler!");
		}

	}

	VulkanSSSS::VulkanSSSS(GLFWwindow* glfwwindow, VulkanEngine* vlkengine)
	:VulkanShadowMap(glfwwindow,vlkengine){

	}


	void VulkanSSSS::CreateOffscreenLightFrameBuffer()
	{
		// Create frame buffer
		std::array<VkImageView, 2> attachments = {
			offscreenLightPass.imageView,
			depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = offscreenLightPass.renderPass;
		framebufferInfo.attachmentCount = attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = offscreenLightPass.width;
		framebufferInfo.height = offscreenLightPass.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &offscreenLightPass.frameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create offscreen framebuffer!");
		}
	}

	void VulkanSSSS::CreateOffscreenLightUniformBuffers()
	{
		//dynamic
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		size_t minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;
		offscreenLightUniformBuffer.dynamicAlignment = sizeof(glm::mat4);
		if (minUboAlignment > 0) {
			offscreenLightUniformBuffer.dynamicAlignment =
				(offscreenLightUniformBuffer.dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		VkDeviceSize bufferSize = engine->modelManager->instanceModelMatrix.size() * offscreenLightUniformBuffer.dynamicAlignment;
		offscreenLightUniformBuffer.uboDynamic.model = (glm::mat4*)alignedAlloc(bufferSize, offscreenLightUniformBuffer.dynamicAlignment);

		offscreenLightUniformBuffer.dynamicUniformData.resize(MAX_FRAMES_IN_FLIGHT);

		//normal
		offscreenLightUniformBuffer.normalUBOAlignment = sizeof(MVPMatrix);
		if (minUboAlignment > 0) {
			offscreenLightUniformBuffer.normalUBOAlignment = (offscreenLightUniformBuffer.normalUBOAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		bufferSize += offscreenLightUniformBuffer.normalUBOAlignment;

		offscreenLightUniformBuffer.uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		offscreenLightUniformBuffer.uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

		VkDeviceSize fragbufferSize = sizeof(FragUniform);
		offscreenLightUniformBuffer.fraguniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		offscreenLightUniformBuffer.fraguniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		offscreenLightUniformBuffer.fraguniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, offscreenLightUniformBuffer.uniformBuffers[i], offscreenLightUniformBuffer.uniformBuffersMemory[i]);
			vkMapMemory(device, offscreenLightUniformBuffer.uniformBuffersMemory[i], 0, engine->modelManager->instanceModelMatrix.size() * offscreenLightUniformBuffer.dynamicAlignment, 0, &offscreenLightUniformBuffer.dynamicUniformData[i]);

			CreateBuffer(fragbufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, offscreenLightUniformBuffer.fraguniformBuffers[i], offscreenLightUniformBuffer.fraguniformBuffersMemory[i]);
			vkMapMemory(device, offscreenLightUniformBuffer.fraguniformBuffersMemory[i], 0, fragbufferSize, 0, &offscreenLightUniformBuffer.fraguniformBuffersMapped[i]);
		}
	}

	void VulkanSSSS::CreateOffscreenLightDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding uboDynamicLayoutBinding = {};
		uboDynamicLayoutBinding.binding = 1;
		uboDynamicLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		uboDynamicLayoutBinding.descriptorCount = 1;
		uboDynamicLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 2;
		samplerLayoutBinding.descriptorCount = 7;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding fraguboLayoutBinding{};
		fraguboLayoutBinding.binding = 3;
		fraguboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		fraguboLayoutBinding.descriptorCount = 1;
		fraguboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 4> bindings =
		{ uboLayoutBinding,uboDynamicLayoutBinding ,samplerLayoutBinding,fraguboLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &offscreenLightDescriptor.descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanSSSS::CreateOffscreenLightDescriptorSets()
	{
		// descriptor pool
		std::array<VkDescriptorPoolSize, 4> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = 7 * static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &offscreenLightDescriptor.descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

		// descriptor sets
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, offscreenLightDescriptor.descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = offscreenLightDescriptor.descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		offscreenLightDescriptor.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, offscreenLightDescriptor.descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = offscreenLightUniformBuffer.uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = offscreenLightUniformBuffer.normalUBOAlignment;

			VkDescriptorBufferInfo dynamicBufferInfo = {};
			dynamicBufferInfo.buffer = offscreenLightUniformBuffer.uniformBuffers[i];
			dynamicBufferInfo.offset = offscreenLightUniformBuffer.normalUBOAlignment;
			dynamicBufferInfo.range = offscreenLightUniformBuffer.dynamicAlignment;

			VkDescriptorImageInfo imageInfo[7];
			imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[0].imageView = textureImageView;
			imageInfo[0].sampler = textureSampler;
			for (int i = 0; i < 6; i++) {
				imageInfo[i + 1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo[i + 1].imageView = pbrTextureImageView[i];
				imageInfo[i + 1].sampler = pbrTextureSampler[i];
			}

			VkDescriptorBufferInfo fragbufferInfo{};
			fragbufferInfo.buffer = offscreenLightUniformBuffer.fraguniformBuffers[i];
			fragbufferInfo.offset = 0;
			fragbufferInfo.range = sizeof(FragUniform);

			std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = offscreenLightDescriptor.descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = offscreenLightDescriptor.descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &dynamicBufferInfo;
			descriptorWrites[1].pTexelBufferView = nullptr;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = offscreenLightDescriptor.descriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 7;
			descriptorWrites[2].pImageInfo = imageInfo;

			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = offscreenLightDescriptor.descriptorSets[i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pBufferInfo = &fragbufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
				descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanSSSS::CreateOffscreenLightPipeline()
	{
		auto vertShaderCode = FileService::ReadFile(SHADERDIR+ "pbr.vert.spv");
		auto fragShaderCode = FileService::ReadFile(SHADERDIR + "pbr.frag.spv");

		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		/*Dynamic state*/
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		/*The VkPipelineVertexInputStateCreateInfo structure describes
		he format of the vertex data that will be passed to the vertex shader*/
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		/*The VkPipelineInputAssemblyStateCreateInfo struct describes two things:
		what kind of geometry will be drawn from the vertices and if primitive restart should be enabled.*/
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		/*A viewport basically describes the region of the framebuffer that the output will be rendered to. */
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		/*Rasterizer*/
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		/*Multisample*/
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		/*Depth and stencil testing*/
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		/*Color blending*/
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		/*Pipeline layout*/
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; // Optional
		pipelineLayoutInfo.pSetLayouts = &(offscreenLightDescriptor.descriptorSetLayout); // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr;// Optional
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &offscreenLightPipeline.pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		/*GraphicsPipeline*/
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = offscreenLightPipeline.pipelineLayout;
		pipelineInfo.renderPass = offscreenLightPass.renderPass;
		pipelineInfo.subpass = 0;
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &offscreenLightPipeline.pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	void VulkanSSSS::DestroyOffscreenLightResources()
	{
		vkDestroySampler(device, offscreenLightPass.sampler, nullptr);
		vkDestroyImageView(device, offscreenLightPass.imageView, nullptr);
		vkDestroyImage(device, offscreenLightPass.image, nullptr);
		vkFreeMemory(device, offscreenLightPass.deviceMemory, nullptr);
		vkDestroyRenderPass(device, offscreenLightPass.renderPass, nullptr);
		vkDestroyDescriptorPool(device, offscreenLightDescriptor.descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, offscreenLightDescriptor.descriptorSetLayout, nullptr);
		vkDestroyPipeline(device, offscreenLightPipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(device, offscreenLightPipeline.pipelineLayout, nullptr);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device, offscreenLightUniformBuffer.uniformBuffers[i], nullptr);
			vkFreeMemory(device, offscreenLightUniformBuffer.uniformBuffersMemory[i], nullptr);
		}
		vkDestroyFramebuffer(device, offscreenLightPass.frameBuffer, nullptr);
	}

	void VulkanSSSS::StartVulkan()
	{

		VulkanShadowMap::StartVulkan();		

		CreateOffscreenLightImage();
		CreateOffscreenLightRenderpass();
		CreateOffscreenLightFrameBuffer();
		CreateOffscreenLightUniformBuffers();
		CreateOffscreenLightDescriptorSetLayout();
		CreateOffscreenLightPipeline();
		CreateOffscreenLightDescriptorSets();

	}

	void VulkanSSSS::ShutDownVulkan()
	{
		DestroyOffscreenLightResources();

		VulkanShadowMap::ShutDownVulkan();
	}

	void VulkanSSSS::RecordCommandBuffer(uint32_t imageIndex, uint32_t currentFrame)
	{
		VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// shadow pass
		{
			std::array<VkClearValue, 1> clearValues{};
			clearValues[0].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = offscreenShadowPass.renderPass;
			renderPassInfo.framebuffer = offscreenShadowPass.frameBuffer;
			renderPassInfo.renderArea.extent.width = offscreenShadowPass.width;
			renderPassInfo.renderArea.extent.height = offscreenShadowPass.height;
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(offscreenShadowPass.width);
			viewport.height = static_cast<float>(offscreenShadowPass.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent.width = offscreenShadowPass.width;
			scissor.extent.height = offscreenShadowPass.height;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdSetDepthBias(
				commandBuffer,
				depthBiasConstant,
				0.0f,
				depthBiasSlope);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenShadowPipeline.Pipeline);

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			for (uint32_t j = 0; j < engine->modelManager->instanceModelMatrix.size(); ++j)
			{
				uint32_t dynamicOffset = j * static_cast<uint32_t>(offscreenShadowUniformBuffer.dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenShadowPipeline.pipelineLayout, 0, 1, &offscreenShadowDescriptor.descriptorSets[currentFrame], 1, &dynamicOffset);
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(engine->modelManager->indices.size()), engine->modelManager->instanceModelMatrix.size(), 0, 0, 0);
			}

			vkCmdEndRenderPass(commandBuffer);
		}

		// light pass
		{
			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = offscreenLightPass.renderPass;
			renderPassInfo.framebuffer = offscreenLightPass.frameBuffer;
			renderPassInfo.renderArea.extent.width = offscreenLightPass.width;
			renderPassInfo.renderArea.extent.height = offscreenLightPass.height;
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenLightPipeline.pipeline);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(offscreenLightPass.width);
			viewport.height = static_cast<float>(offscreenLightPass.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent.width = offscreenLightPass.width;
			scissor.extent.height = offscreenLightPass.height;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			for (uint32_t j = 0; j < engine->modelManager->instanceModelMatrix.size(); ++j)
			{
				uint32_t dynamicOffset = j * static_cast<uint32_t>(offscreenLightUniformBuffer.dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenLightPipeline.pipelineLayout, 0, 1, &offscreenLightDescriptor.descriptorSets[currentFrame], 1, &dynamicOffset);
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(engine->modelManager->indices.size()), engine->modelManager->instanceModelMatrix.size(), 0, 0, 0);
			}

			vkCmdEndRenderPass(commandBuffer);
		}

		// main pass
		{
			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(swapChainExtent.width);
			viewport.height = static_cast<float>(swapChainExtent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = swapChainExtent;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			for (uint32_t j = 0; j < engine->modelManager->instanceModelMatrix.size(); ++j)
			{
				uint32_t dynamicOffset = j * static_cast<uint32_t>(dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 1, &dynamicOffset);
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(engine->modelManager->indices.size()), engine->modelManager->instanceModelMatrix.size(), 0, 0, 0);
			}

			vkCmdEndRenderPass(commandBuffer);
		}

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void VulkanSSSS::UpdateUniformBuffer(uint32_t currentImage)
	{
		// offscreen pass
		MVPMatrix offscreenubo{};
		offscreenubo.view = glm::lookAt(engine->lightPosition, glm::vec3(0, 0, 0), engine->worldUp);
		offscreenubo.proj = glm::perspective(glm::radians(engine->lightFov), 1.0f, shadowMapZNear, shadowMapZFar);
		offscreenubo.proj[1][1] *= -1;

		memcpy(offscreenShadowUniformBuffer.dynamicUniformData[currentImage], &offscreenubo, sizeof(offscreenubo));

		uint32_t index = 0;
		for (int i = 0; i < engine->modelManager->instanceModelMatrix.size(); i++)
		{
			glm::mat4* modelMat = (glm::mat4*)(((uint64_t)offscreenShadowUniformBuffer.uboDynamic.model + (index * offscreenShadowUniformBuffer.dynamicAlignment)));
			*modelMat = engine->modelManager->instanceModelMatrix[i];
			++index;
		}

		void* offscreendata = reinterpret_cast<size_t*>(offscreenShadowUniformBuffer.dynamicUniformData[currentImage]) + offscreenShadowUniformBuffer.normalUBOAlignment / sizeof(size_t);
		memcpy(offscreendata, offscreenShadowUniformBuffer.uboDynamic.model, engine->modelManager->instanceModelMatrix.size() * offscreenShadowUniformBuffer.dynamicAlignment);

		// main pass
		ShadowMVP lightubo{};
		glm::mat4 worldview = engine->camera->GetViewMatrix();
		glm::mat4 worldproj = glm::perspective(engine->camera->Fov, swapChainExtent.width / (float)(swapChainExtent.height), engine->camera->zNear, engine->camera->zFar);
		worldproj[1][1] *= -1;
		lightubo.worldprojview = worldproj * worldview;
		lightubo.lightprojview = offscreenubo.proj * offscreenubo.view;

		FragUniform fragubo{};
		fragubo.viewPosition = engine->camera->camPosition;
		fragubo.lightPosition = engine->lightPosition;
		fragubo.lightRadiance = engine->lightRadiance;

		memcpy(dynamicUniformData[currentImage], &lightubo, sizeof(lightubo));
		memcpy(fraguniformBuffersMapped[currentImage], &fragubo, sizeof(fragubo));
		
		index = 0;
		for (int i = 0; i < engine->modelManager->instanceModelMatrix.size(); i++)
		{
			glm::mat4* modelMat = (glm::mat4*)(((uint64_t)uboDynamic.model + (index * dynamicAlignment)));
			*modelMat = engine->modelManager->instanceModelMatrix[i];
			++index;
		}
		void* lightdata = reinterpret_cast<size_t*>(dynamicUniformData[currentImage]) + normalUBOAlignment / sizeof(size_t);
		memcpy(lightdata, uboDynamic.model, engine->modelManager->instanceModelMatrix.size() * dynamicAlignment);

		// light
		memcpy(offscreenLightUniformBuffer.dynamicUniformData[currentImage], &lightubo, sizeof(lightubo));
		memcpy(offscreenLightUniformBuffer.fraguniformBuffersMapped[currentImage], &fragubo, sizeof(fragubo));

		index = 0;
		for (int i = 0; i < engine->modelManager->instanceModelMatrix.size(); i++)
		{
			glm::mat4* modelMat = (glm::mat4*)(((uint64_t)offscreenLightUniformBuffer.uboDynamic.model + (index * offscreenLightUniformBuffer.dynamicAlignment)));
			*modelMat = engine->modelManager->instanceModelMatrix[i];
			++index;
		}
		void* lightdata2 = reinterpret_cast<size_t*>(offscreenLightUniformBuffer.dynamicUniformData[currentImage]) + offscreenLightUniformBuffer.normalUBOAlignment / sizeof(size_t);
		memcpy(lightdata2, offscreenLightUniformBuffer.uboDynamic.model, engine->modelManager->instanceModelMatrix.size() * offscreenLightUniformBuffer.dynamicAlignment);
	}
}