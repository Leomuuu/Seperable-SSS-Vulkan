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
    void VulkanEngine::InitEngine()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

        vulkanSetup = new VulkanSetup(window);
        renderPipline = new RenderPipline(vulkanSetup->device);
        renderBuffer = new RenderBuffer(vulkanSetup,renderPipline);
    }
    void VulkanEngine::StartEngine()
    {
        vulkanSetup->InitVulkan();
        renderPipline->CreateRenderPass(vulkanSetup->swapChainImageFormat);
        renderPipline->CreateGraphicsPipeline(
            std::string("C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/shader/simple_shader.vert.spv"),
            std::string("C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/shader/simple_shader.frag.spv"));
        renderBuffer->CreateBuffers();
    }
    void VulkanEngine::MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

        }
    }
    void VulkanEngine::ShutDownEngine()
    {
        renderBuffer->DestroyBuffers();
        renderPipline->DestroyGraphicsPipeline();
        renderPipline->DestroyRenderPass();
        vulkanSetup->ShutDownVulkan();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

	
	void VulkanEngine::DrawFrame()
	{

	}
}
