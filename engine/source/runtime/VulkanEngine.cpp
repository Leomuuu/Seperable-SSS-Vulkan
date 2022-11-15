#include "VulkanEngine.h"
namespace VlkEngine {
    VulkanEngine::VulkanEngine(int w, int h, std::string name):width(w),height(h),windowName(name)
    {
        InitEngine();
    }
    VulkanEngine::~VulkanEngine()
    {
        delete vulkanSetup;
    }
    void VulkanEngine::Run()
    {
        StartEngine();
        MainLoop();
        ShutDownEngine();
    }

	void VulkanEngine::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
        auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
	}

	void VulkanEngine::MouseMovecallback(GLFWwindow* window, double xpos, double ypos)
	{
        auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
        InputSystem* inputSystem = app->inputSystem;

        if (inputSystem!=nullptr) {
            inputSystem->MouseMovement(window, xpos, ypos);
        }
	}

	void VulkanEngine::InitEngine()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback); 

        camera = new Camera(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::radians(45.0f),0.1f,10.0f);

        inputSystem = new InputSystem(camera);
        glfwSetCursorPosCallback(window, MouseMovecallback);

        vulkanSetup = new VulkanSetup(window);
        renderDescriptor = new RenderDescriptor(vulkanSetup);
        renderPipline = new RenderPipline(renderDescriptor);
        renderBuffer = new RenderBuffer(vulkanSetup,renderPipline);
        renderImage = new RenderImage(renderBuffer);
        vulkanSyncObject = new VulkanSyncObject(vulkanSetup);
        
    }
    void VulkanEngine::StartEngine()
    {
        vulkanSetup->InitVulkan();
        renderPipline->CreateRenderPass(vulkanSetup->swapChainImageFormat);
        renderDescriptor->CreateDescriptorSetLayout();
        renderPipline->CreateGraphicsPipeline(
            std::string("C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/shader/simple_shader.vert.spv"),
            std::string("C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/shader/simple_shader.frag.spv"));
        renderBuffer->CreateBuffers();
        renderImage->CreateTextureImage();
        renderImage->CreateTextureImageView();
        renderImage->CreateTextureSampler();
        renderDescriptor->CreateDescriptorPool();
        renderDescriptor->CreateDescriptorSets(renderBuffer,renderImage);
        vulkanSyncObject->CreateSyncObjects();
    }
    void VulkanEngine::MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            inputSystem->ProcessInput(window);
            DrawFrame();
        }
        vkDeviceWaitIdle(vulkanSetup->device);
    }
    void VulkanEngine::ShutDownEngine()
    {
        vulkanSyncObject->DestroySyncObjects();
        renderDescriptor->DestroyDescriptor();
        renderImage->DestroyTextureSampler();
        renderImage->DestroyTextureImageView();
        renderImage->DestroyTextureImage();
        renderBuffer->DestroyBuffers();
        renderPipline->DestroyGraphicsPipeline();
        renderPipline->DestroyRenderPass();
        vulkanSetup->ShutDownVulkan();
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }

	
	void VulkanEngine::DrawFrame()
	{
        vkWaitForFences(vulkanSetup->device, 1, &(vulkanSyncObject->inFlightFences[currentFrame]), VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(vulkanSetup->device, vulkanSetup->swapChain,
            UINT64_MAX, vulkanSyncObject->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            WindowSurfaceChange();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        UpdateUniformBuffer(currentFrame);
        
        vkResetFences(vulkanSetup->device, 1, &(vulkanSyncObject->inFlightFences[currentFrame]));
        
        vkResetCommandBuffer(renderBuffer->commandBuffers[currentFrame], 0);
        RecordCommandBuffer(imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { vulkanSyncObject->imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(renderBuffer->commandBuffers[currentFrame]);
        VkSemaphore signalSemaphores[] = { vulkanSyncObject->renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(vulkanSetup->graphicsQueue, 1, &submitInfo, vulkanSyncObject->inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { vulkanSetup->swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(vulkanSetup->presentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            WindowSurfaceChange();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

	void VulkanEngine::RecordCommandBuffer(uint32_t imageIndex)
	{
        VkCommandBuffer commandBuffer = renderBuffer->commandBuffers[currentFrame];

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPipline->renderPass;
        renderPassInfo.framebuffer = renderBuffer->swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = vulkanSetup->swapChainExtent;
        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPipline->graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(vulkanSetup->swapChainExtent.width);
        viewport.height = static_cast<float>(vulkanSetup->swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = vulkanSetup->swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { renderBuffer->vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, renderBuffer->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPipline->pipelineLayout,
            0, 1, &((renderDescriptor->descriptorSets)[currentFrame]), 0, nullptr);

        //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
	}

	void VulkanEngine::WindowSurfaceChange()
	{
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(vulkanSetup->device);

        renderBuffer->DestroyFramebuffers();
        vulkanSetup->DestroyImageViews();
        vulkanSetup->DestroySwapChain();

        vulkanSetup->CreateSwapChain();
        vulkanSetup->CreateImageViews();
        renderBuffer->CreateFramebuffers();

	}

	void VulkanEngine::UpdateUniformBuffer(uint32_t currentImage)
	{
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = camera->GetViewMatrix();
        ubo.proj = glm::perspective(camera->Fov, vulkanSetup->swapChainExtent.width / (float)(vulkanSetup->swapChainExtent.height), camera->zNear, camera->zFar);
        ubo.proj[1][1] *= -1;

        memcpy(renderBuffer->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));


    }
}
