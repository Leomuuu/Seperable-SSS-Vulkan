#include "Editor.h"

#include<windows.h>

namespace VlkEngine {
	Editor::Editor(VulkanEngine* engine) :
		renderEngine(engine)
	{

	}

	void Editor::Run()
	{
		if (renderEngine) {
			StartEditor();
			MainLoop();
			ShutDownEditor();
		}
	}

	void Editor::StartEditor()
	{
		if (renderEngine) {
			renderEngine->StartEngine();
		}
		StartUI();
	}

	void Editor::MainLoop()
	{
		clock_t starttime, finishtime;
		double deltatime;
		starttime = clock();
		while (!glfwWindowShouldClose(renderEngine->window)) {
			glfwPollEvents();
			DrawUI();
			DrawFrame();
			finishtime = clock();
			deltatime = (double)(finishtime - starttime) / CLOCKS_PER_SEC;
			renderEngine->inputSystem->ProcessInput(renderEngine->window, deltatime);
			fps = Calfps(deltatime);
			starttime = clock();
		}

		vkDeviceWaitIdle(renderEngine->vulkanBase->device);
	}

	void Editor::ShutDownEditor()
	{
		ShutdownUI();
		if (renderEngine) 
			renderEngine->ShutDownEngine();
		
	}

	void Editor::StartUI()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		// Initialize some DearImgui specific resources
		CreateUIDescriptorPool();
		CreateUIRenderPass();
		CreateUICommandPool(&uiCommandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		CreateUICommandBuffers();
		CreateUIFramebuffers();

		// Provide bind points from Vulkan API
		ImGui_ImplGlfw_InitForVulkan(renderEngine->window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = renderEngine->vulkanBase->instance;
		init_info.PhysicalDevice = renderEngine->vulkanBase->physicalDevice;
		init_info.Device = renderEngine->vulkanBase->device;
		init_info.QueueFamily = renderEngine->vulkanBase->indices.graphicsFamily.value();
		init_info.Queue = renderEngine->vulkanBase->graphicsQueue;
		init_info.DescriptorPool = uiDescriptorPool;
		init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
		ImGui_ImplVulkan_Init(&init_info, uiRenderPass);

		// Upload the fonts for DearImgui
		VkCommandBuffer commandBuffer = renderEngine->vulkanBase->PreSingleTimeCommands(uiCommandPool);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		renderEngine->vulkanBase->PostSingleTimeCommands(commandBuffer, uiCommandPool);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void Editor::ShutdownUI()
	{
		void DestroyUIDescriptorPool();
		void DestroyUICommandPool();
		void DestroyUIFramebuffers();
		void DestroyUIRenderPass();
	}

	void Editor::CreateUIDescriptorPool()
	{
		VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;
		if (vkCreateDescriptorPool(renderEngine->vulkanBase->device, &pool_info, nullptr, &uiDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Cannot allocate UI descriptor pool!");
		}
	}

	void Editor::DestroyUIDescriptorPool()
	{
		vkDestroyDescriptorPool(renderEngine->vulkanBase->device, uiDescriptorPool, nullptr);
	}

	void Editor::CreateUIRenderPass()
	{
		// Create an attachment description for the render pass
		VkAttachmentDescription attachmentDescription = {};
		attachmentDescription.format = renderEngine->vulkanBase->swapChainImageFormat;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // Need UI to be drawn on top of main
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Last pass so we want to present after
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// Create a color attachment reference
		VkAttachmentReference attachmentReference = {};
		attachmentReference.attachment = 0;
		attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Create a subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &attachmentReference;

		// Create a subpass dependency to synchronize our main and UI render passes
		// We want to render the UI after the geometry has been written to the framebuffer
		// so we need to configure a subpass dependency as such
		VkSubpassDependency subpassDependency = {};
		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Create external dependency
		subpassDependency.dstSubpass = 0; // The geometry subpass comes first
		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Wait on writes
		subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// Finally create the UI render pass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachmentDescription;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &subpassDependency;

		if (vkCreateRenderPass(renderEngine->vulkanBase->device, &renderPassInfo, nullptr, &uiRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("Unable to create UI render pass!");
		}
	}

	void Editor::DestroyUIRenderPass()
	{
		vkDestroyRenderPass(renderEngine->vulkanBase->device, uiRenderPass, nullptr);

	} 

	void Editor::CreateUICommandPool(VkCommandPool* cmdPool, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.queueFamilyIndex = renderEngine->vulkanBase->indices.graphicsFamily.value();
		commandPoolCreateInfo.flags = flags;

		if (vkCreateCommandPool(renderEngine->vulkanBase->device, &commandPoolCreateInfo, nullptr, cmdPool) != VK_SUCCESS) {
			throw std::runtime_error("Could not create graphics command pool!");
		}
	}

	void Editor::DestroyUICommandPool()
	{
		vkDestroyCommandPool(renderEngine->vulkanBase->device, uiCommandPool, nullptr);
	}

	void Editor::CreateUICommandBuffers()
	{
		uiCommandBuffers.resize(renderEngine->vulkanBase->swapChainImageViews.size());

		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = uiCommandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(uiCommandBuffers.size());

		if (vkAllocateCommandBuffers(renderEngine->vulkanBase->device, &commandBufferAllocateInfo, uiCommandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Unable to allocate UI command buffers!");
		}
	}

	void Editor::CreateUIFramebuffers()
	{
		// Create some UI frame buffers. These will be used in the render pass for the UI
		uiFramebuffers.resize(renderEngine->vulkanBase->swapChainImages.size());
		VkImageView attachment[1];
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = uiRenderPass;
		info.attachmentCount = 1;
		info.pAttachments = attachment;
		info.width = renderEngine->vulkanBase->swapChainExtent.width;
		info.height = renderEngine->vulkanBase->swapChainExtent.height;
		info.layers = 1;
		for (uint32_t i = 0; i < renderEngine->vulkanBase->swapChainImages.size(); ++i) {
			attachment[0] = renderEngine->vulkanBase->swapChainImageViews[i];
			if (vkCreateFramebuffer(renderEngine->vulkanBase->device, &info, nullptr, &uiFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Unable to create UI frame buffers!");
			}
		}
	}

	void Editor::DestroyUIFramebuffers()
	{
		for (auto framebuffer : uiFramebuffers) {
			vkDestroyFramebuffer(renderEngine->vulkanBase->device, framebuffer, nullptr);
		}
	}

	void Editor::DrawUI()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		static glm::vec3 lightpos = renderEngine->lightPosition;
		static float lightposx = lightpos.x;
		static float lightposy = lightpos.y;
		static float lightposz = lightpos.z;
		ImGui::Begin("Renderer Information");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Light Position");
		ImGui::SliderFloat("lightposx", &lightposx, -5.0f, 5.0f);
		ImGui::SliderFloat("lightposy", &lightposy, -5.0f, 5.0f);
		ImGui::SliderFloat("lightposz", &lightposz, -5.0f, 5.0f);
		renderEngine->lightPosition=glm::vec3(lightposx, lightposy, lightposz);

		ImGui::TextColored(ImVec4(1, 1, 0, 1),"Camera Position");
		ImGui::Text("x: %.3f", renderEngine->camera->GetPosition().x);
		ImGui::Text("y: %.3f", renderEngine->camera->GetPosition().y);
		ImGui::Text("z: %.3f", renderEngine->camera->GetPosition().z);

		ImGui::TextColored(ImVec4(1, 1, 0, 1), "FrameRate");
		ImGui::Text("%.3f fps",fps);
		ImGui::End();

		ImGui::Render();
	}

	void Editor::RecordUICommandBuffer(uint32_t bufferIndex)
	{
		VkCommandBufferBeginInfo cmdBufferBegin = {};
		cmdBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBegin.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VkCommandBuffer commandBuffer = uiCommandBuffers[renderEngine->currentFrame];

		if (vkBeginCommandBuffer(commandBuffer, &cmdBufferBegin) != VK_SUCCESS) {
			throw std::runtime_error("Unable to start recording UI command buffer!");
		}

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = uiRenderPass;
		renderPassBeginInfo.framebuffer = uiFramebuffers[bufferIndex];
		renderPassBeginInfo.renderArea.extent.width = renderEngine->vulkanBase->swapChainExtent.width;
		renderPassBeginInfo.renderArea.extent.height = renderEngine->vulkanBase->swapChainExtent.height;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Grab and record the draw data for Dear Imgui
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		// End and submit render pass
		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffers!");
		}
	}

	void Editor::WindowSurfaceChange()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(renderEngine->vulkanBase->window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(renderEngine->vulkanBase->window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(renderEngine->vulkanBase->device);

		renderEngine->vulkanBase->DestroyDepthResource();
		renderEngine->vulkanBase->DestroyFramebuffers();
		DestroyUIFramebuffers();
		renderEngine->vulkanBase->DestroyImageViews();
		renderEngine->vulkanBase->DestroySwapChain();

		renderEngine->vulkanBase->CreateSwapChain();
		renderEngine->vulkanBase->CreateImageViews();
		renderEngine->vulkanBase->CreateDepthResource();
		renderEngine->vulkanBase->CreateFramebuffers();
		CreateUIFramebuffers();
	}

	void Editor::DrawFrame()
	{
		vkWaitForFences(renderEngine->vulkanBase->device, 1, &(renderEngine->vulkanBase->inFlightFences[renderEngine->currentFrame]), VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(renderEngine->vulkanBase->device, renderEngine->vulkanBase->swapChain,
			UINT64_MAX, renderEngine->vulkanBase->imageAvailableSemaphores[renderEngine->currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {

			WindowSurfaceChange();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(renderEngine->vulkanBase->device, 1, &(renderEngine->vulkanBase->inFlightFences[renderEngine->currentFrame]));

		renderEngine->vulkanBase->UpdateUniformBuffer(renderEngine->currentFrame);
		vkResetCommandBuffer(renderEngine->vulkanBase->commandBuffers[renderEngine->currentFrame], 0);
		renderEngine->vulkanBase->RecordCommandBuffer(imageIndex, renderEngine->currentFrame);
		vkResetCommandBuffer(uiCommandBuffers[renderEngine->currentFrame], 0);
		RecordUICommandBuffer(imageIndex);

		std::array<VkCommandBuffer, 2> cmdbuffers{renderEngine->vulkanBase->commandBuffers[renderEngine->currentFrame]
			,uiCommandBuffers[renderEngine->currentFrame] };
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { renderEngine->vulkanBase->imageAvailableSemaphores[renderEngine->currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = static_cast<uint32_t>(cmdbuffers.size());
		submitInfo.pCommandBuffers = cmdbuffers.data();

		VkSemaphore signalSemaphores[] = { renderEngine->vulkanBase->renderFinishedSemaphores[renderEngine->currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		if (vkQueueSubmit(renderEngine->vulkanBase->graphicsQueue, 1, &submitInfo, renderEngine->vulkanBase->inFlightFences[renderEngine->currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { renderEngine->vulkanBase->swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR(renderEngine->vulkanBase->presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || renderEngine->framebufferResized) {
			renderEngine->framebufferResized = false;
			WindowSurfaceChange();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		renderEngine->currentFrame = (renderEngine->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	float Editor::Calfps(float DeltaTime)
	{
		static float avgDuration = 0.f;
		static float alpha = 1.f / 100.f; // 采样数设置为100
		static int frameCount = 0;

		++frameCount;

		int fps = 0;
		if (frameCount==1){
			avgDuration = DeltaTime;
		}
		else{
			avgDuration = avgDuration * (1 - alpha) + DeltaTime * alpha;
		}
		return (1.f / avgDuration);
	}
}
