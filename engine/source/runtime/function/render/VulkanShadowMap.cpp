#include "VulkanShadowMap.h"
#include "../../VulkanEngine.h"

namespace VlkEngine {
	
	void VulkanShadowMap::CreateOffscreenRenderpass()
	{
		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = DEPTH_FORMAT;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;				
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;												
		subpass.pDepthStencilAttachment = &depthReference;						

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCreateInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &offscreenShadowPass.renderPass)!=VK_SUCCESS) {
			throw std::runtime_error("failed to create offscreen render pass!");
		}
	}

	void VulkanShadowMap::CreateOffscreenImage()
	{
		offscreenShadowPass.width = SHADOWMAP_DIMENSION;
		offscreenShadowPass.height = SHADOWMAP_DIMENSION;

		// offscreen image imageview
		CreateImage(offscreenShadowPass.width, offscreenShadowPass.height,
			DEPTH_FORMAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, offscreenShadowPass.image, offscreenShadowPass.deviceMemory);
		offscreenShadowPass.imageView=CreateImageView(offscreenShadowPass.image, DEPTH_FORMAT, VK_IMAGE_ASPECT_DEPTH_BIT);

		// Used to sample in the fragment shader for shadowed rendering
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
		if (vkCreateSampler(device, &samplerInfo, nullptr, &offscreenShadowPass.depthSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create offscreen sampler!");
		}
	}

	void VulkanShadowMap::CreateOffscreenFrameBuffer()
	{
		// Create frame buffer
		VkFramebufferCreateInfo framebufferInfo{};

		framebufferInfo.renderPass = offscreenShadowPass.renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &offscreenShadowPass.imageView;
		framebufferInfo.width = offscreenShadowPass.width;
		framebufferInfo.height = offscreenShadowPass.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &offscreenShadowPass.frameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create offscreen framebuffer!");
		}
	}

	void VulkanShadowMap::CreateOffscreenUniformBuffers()
	{
		//dynamic
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		size_t minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;
		offscreenShadowUniformBuffer.dynamicAlignment = sizeof(glm::mat4);
		if (minUboAlignment > 0) {
			offscreenShadowUniformBuffer.dynamicAlignment = (offscreenShadowUniformBuffer.dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		VkDeviceSize bufferSize = engine->modelManager->instanceModelMatrix.size() * offscreenShadowUniformBuffer.dynamicAlignment;
		offscreenShadowUniformBuffer.uboDynamic.model = (glm::mat4*)alignedAlloc(bufferSize, offscreenShadowUniformBuffer.dynamicAlignment);

		offscreenShadowUniformBuffer.dynamicUniformData.resize(MAX_FRAMES_IN_FLIGHT);

		//normal
		offscreenShadowUniformBuffer.normalUBOAlignment = sizeof(MVPMatrix);
		if (minUboAlignment > 0) {
			offscreenShadowUniformBuffer.normalUBOAlignment = (offscreenShadowUniformBuffer.normalUBOAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		bufferSize += offscreenShadowUniformBuffer.normalUBOAlignment;

		offscreenShadowUniformBuffer.uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		offscreenShadowUniformBuffer.uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, offscreenShadowUniformBuffer.uniformBuffers[i], offscreenShadowUniformBuffer.uniformBuffersMemory[i]);
			vkMapMemory(device, offscreenShadowUniformBuffer.uniformBuffersMemory[i], 0, engine->modelManager->instanceModelMatrix.size() * offscreenShadowUniformBuffer.dynamicAlignment, 0, &offscreenShadowUniformBuffer.dynamicUniformData[i]);
		}
	}

	void VulkanShadowMap::CreateOffscreenDescriptorSetLayout()
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

		std::array<VkDescriptorSetLayoutBinding, 2> bindings =
		{ uboLayoutBinding,uboDynamicLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &offscreenShadowDescriptor.descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanShadowMap::CreateOffscreenDescriptorSets()
	{
		// Descriptor pool
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &offscreenShadowDescriptor.descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

		// Descriptor sets
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, offscreenShadowDescriptor.descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = offscreenShadowDescriptor.descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		offscreenShadowDescriptor.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, offscreenShadowDescriptor.descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = offscreenShadowUniformBuffer.uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = offscreenShadowUniformBuffer.normalUBOAlignment;

			VkDescriptorBufferInfo dynamicBufferInfo = {};
			dynamicBufferInfo.buffer = offscreenShadowUniformBuffer.uniformBuffers[i];
			dynamicBufferInfo.offset = offscreenShadowUniformBuffer.normalUBOAlignment;
			dynamicBufferInfo.range = offscreenShadowUniformBuffer.dynamicAlignment;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = offscreenShadowDescriptor.descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = offscreenShadowDescriptor.descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &dynamicBufferInfo;
			descriptorWrites[1].pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
				descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanShadowMap::CreateOffscreenPipeline()
	{
		// vertex shader only
		auto vertShaderCode = FileService::ReadFile("C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/shader/offscreen.vert.spv");
		

		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo };

		/*Dynamic state*/
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_DEPTH_BIAS
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
		rasterizer.depthClampEnable = VK_FALSE; // can be used in shadow map
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_TRUE; // can be used in shadow map
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

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
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;


		/*Color blending*/
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 0;
		colorBlending.pAttachments = nullptr;

		/*Pipeline layout*/
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; 
		pipelineLayoutInfo.pSetLayouts = &(offscreenShadowDescriptor.descriptorSetLayout);
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &offscreenShadowPipeline.pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		/*GraphicsPipeline*/
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 1;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = offscreenShadowPipeline.pipelineLayout;
		pipelineInfo.renderPass = offscreenShadowPass.renderPass;
		pipelineInfo.subpass = 0;
		// creating a new graphics pipeline by deriving from an existing pipeline is allowed in Vulkan
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional   
		pipelineInfo.basePipelineIndex = -1; // Optional
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &offscreenShadowPipeline.Pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	VulkanShadowMap::VulkanShadowMap(GLFWwindow* glfwwindow, VulkanEngine* vlkengine):
		VulkanPBR(glfwwindow,vlkengine)
	{
		lightVertPath = SHADERDIR+"pbr_shadowmap.vert.spv";
		lightFragPath = SHADERDIR+"pbr_shadowmap.frag.spv";
	}

	void VulkanShadowMap::UpdateUniformBuffer(uint32_t currentImage)
	{
		

		// offscreen pass
		MVPMatrix offscreenubo{};
		offscreenubo.view = glm::lookAt(engine->lightPosition, glm::vec3(0,0,0), engine->worldUp);
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
	
		
		// light pass
		ShadowMVP lightubo{};
		glm::mat4 worldview = engine->camera->GetViewMatrix();
		glm::mat4 worldproj= glm::perspective(engine->camera->Fov, swapChainExtent.width / (float)(swapChainExtent.height), engine->camera->zNear, engine->camera->zFar);
		worldproj[1][1] *= -1;
		lightubo.worldprojview = worldproj * worldview;
		lightubo.lightprojview =offscreenubo.proj * offscreenubo.view;

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
	
	}



	void VulkanShadowMap::StartVulkan()
	{		
		CreateOffscreenImage();
		CreateOffscreenRenderpass();
		CreateOffscreenFrameBuffer();
		CreateOffscreenUniformBuffers();
		CreateOffscreenDescriptorSetLayout();
		CreateOffscreenPipeline();
		CreateOffscreenDescriptorSets();
		
		VulkanPBR::StartVulkan();

	}

	void VulkanShadowMap::ShutDownVulkan()
	{
		VulkanPBR::ShutDownVulkan();
	}

	void VulkanShadowMap::RecordCommandBuffer(uint32_t imageIndex, uint32_t currentFrame)
	{
		VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// offscreen pass
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

	void VulkanShadowMap::CreateDescriptorSetLayout()
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

		VkDescriptorSetLayoutBinding offscreensamplerLayoutBinding{};
		offscreensamplerLayoutBinding.binding = 4;
		offscreensamplerLayoutBinding.descriptorCount = 1;
		offscreensamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		offscreensamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 5> bindings =
		{ uboLayoutBinding,uboDynamicLayoutBinding ,samplerLayoutBinding,fraguboLayoutBinding,
		offscreensamplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanShadowMap::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 5> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = 7 * static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[4].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanShadowMap::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = normalUBOAlignment;

			VkDescriptorBufferInfo dynamicBufferInfo = {};
			dynamicBufferInfo.buffer = uniformBuffers[i];
			dynamicBufferInfo.offset = normalUBOAlignment;
			dynamicBufferInfo.range = dynamicAlignment;

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
			fragbufferInfo.buffer = fraguniformBuffers[i];
			fragbufferInfo.offset = 0;
			fragbufferInfo.range = sizeof(FragUniform);

			VkDescriptorImageInfo offscreenimageInfo{};
			offscreenimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			offscreenimageInfo.imageView = offscreenShadowPass.imageView;
			offscreenimageInfo.sampler = offscreenShadowPass.depthSampler;

			std::array<VkWriteDescriptorSet, 5> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &dynamicBufferInfo;
			descriptorWrites[1].pTexelBufferView = nullptr;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = descriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 7;
			descriptorWrites[2].pImageInfo = imageInfo;

			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = descriptorSets[i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pBufferInfo = &fragbufferInfo;

			descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[4].dstSet = descriptorSets[i];
			descriptorWrites[4].dstBinding = 4;
			descriptorWrites[4].dstArrayElement = 0;
			descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[4].descriptorCount = 1;
			descriptorWrites[4].pImageInfo = &offscreenimageInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
				descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanShadowMap::CreateGraphicsPipeline()
	{
		VulkanPBR::CreateGraphicsPipeline();
	}
}