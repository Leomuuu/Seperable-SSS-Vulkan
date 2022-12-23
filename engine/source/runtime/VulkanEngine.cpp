#include "VulkanEngine.h"
namespace VlkEngine {
    VulkanEngine::VulkanEngine(int w, int h, std::string name):width(w),height(h),windowName(name)
    {
        InitEngine();
    }

    VulkanEngine::~VulkanEngine()
    {
        delete vulkanBase;
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

	void VulkanEngine::KeyInputcallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
	}

	void VulkanEngine::InitEngine()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback); 
        glfwSetKeyCallback(window, KeyInputcallback); 
        glfwSetCursorPosCallback(window, MouseMovecallback);

        camera = new Camera(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
            worldUp,
            glm::radians(45.0f),0.1f,10.0f);

        inputSystem = new InputSystem(camera);

        modelManager = new PbrModelManager();
        modelManager->SetModel(
            MODELDIR + "model/Free+Head/OBJ/Head/Head.OBJ",
            MODELDIR + "model/Free+Head/JPGTextures/Head/Colour_8k.jpg");
        modelManager->SetPbrTexture(
            MODELDIR + "model/Free+Head/JPGTextures/Head/Cavity_8k.jpg",
            MODELDIR + "model/Free+Head/JPGTextures/Head/Gloss_8k.jpg",
            MODELDIR + "model/Free+Head/JPGTextures/Head/Spec_8k.jpg",
            MODELDIR + "model/Free+Head/JPGTextures/Head/MicroNormal.jpg",
            MODELDIR + "model/Free+Head/JPGTextures/Head/Normal.jpg",
            MODELDIR + "model/Free+Head/JPGTextures/Head/Translucency.jpg"
        );
        modelManager->instanceModelMatrix.push_back(glm::mat4(1.0f));
        modelManager->instanceModelMatrix.push_back(glm::translate(
            glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), 
                glm::vec3(0.0, 1.0, 0.0)), glm::vec3(0.0f, 0.0f, -0.4f)));
        modelManager->instanceModelMatrix.push_back(glm::translate(
            glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
                glm::vec3(0.0, 1.0, 0.0)), glm::vec3(0.0f, 0.0f, -0.8f)));
        

        vulkanBase = new VulkanSSSS(window, this);
        
    }

    void VulkanEngine::StartEngine()
    {
        vulkanBase->CreateVulkanResources();
        vulkanBase->StartVulkan();
        
    }

    void VulkanEngine::MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            DrawFrame();
            inputSystem->ProcessInput(window,1000);
        }
        vkDeviceWaitIdle(vulkanBase->device);
    }

    void VulkanEngine::ShutDownEngine()
    {
        vulkanBase->ShutDownVulkan();
        vulkanBase->DestroyVulkanResources();
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }

	
	void VulkanEngine::DrawFrame()
	{
        vkWaitForFences(vulkanBase->device, 1, &(vulkanBase->inFlightFences[currentFrame]), VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(vulkanBase->device, vulkanBase->swapChain,
            UINT64_MAX, vulkanBase->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            WindowSurfaceChange();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(vulkanBase->device, 1, &(vulkanBase->inFlightFences[currentFrame]));
        
        vkResetCommandBuffer(vulkanBase->commandBuffers[currentFrame], 0);
        
        vulkanBase->UpdateUniformBuffer(currentFrame);
        vulkanBase->RecordCommandBuffer(imageIndex,currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { vulkanBase->imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(vulkanBase->commandBuffers[currentFrame]);
        VkSemaphore signalSemaphores[] = { vulkanBase->renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(vulkanBase->graphicsQueue, 1, &submitInfo, vulkanBase->inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { vulkanBase->swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(vulkanBase->presentQueue, &presentInfo);
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

        vkDeviceWaitIdle(vulkanBase->device);

        vulkanBase->DestroyDepthResource();
        vulkanBase->DestroyFramebuffers();
        vulkanBase->DestroyImageViews();
        vulkanBase->DestroySwapChain();

        vulkanBase->CreateSwapChain();
        vulkanBase->CreateImageViews();
        vulkanBase->CreateDepthResource();
        vulkanBase->CreateFramebuffers();
	}

}
