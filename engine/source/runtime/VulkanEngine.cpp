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

	void VulkanEngine::InitEngine()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback); 

        vulkanSetup = new VulkanSetup(window);
        renderPipline = new RenderPipline(vulkanSetup->device);
        renderBuffer = new RenderBuffer(vulkanSetup,renderPipline);
        vulkanSyncObject = new VulkanSyncObject(vulkanSetup->device);
    }
    void VulkanEngine::StartEngine()
    {
        vulkanSetup->InitVulkan();
        renderPipline->CreateRenderPass(vulkanSetup->swapChainImageFormat);
        renderPipline->CreateGraphicsPipeline(
            std::string("C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/shader/simple_shader.vert.spv"),
            std::string("C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/shader/simple_shader.frag.spv"));
        renderBuffer->CreateBuffers();
        vulkanSyncObject->CreateSyncObjects();
    }
    void VulkanEngine::MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            DrawFrame();
        }
        vkDeviceWaitIdle(vulkanSetup->device);
    }
    void VulkanEngine::ShutDownEngine()
    {
        vulkanSyncObject->DestroySyncObjects();
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
        
        vkResetFences(vulkanSetup->device, 1, &(vulkanSyncObject->inFlightFences[currentFrame]));
        
        vkResetCommandBuffer(renderBuffer->commandBuffers[currentFrame], 0);
        renderBuffer->RecordCommandBuffer(renderBuffer->commandBuffers[currentFrame], imageIndex);

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

}
