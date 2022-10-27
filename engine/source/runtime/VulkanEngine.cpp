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
    }
    void VulkanEngine::StartEngine()
    {
        vulkanSetup->InitVulkan();
    }
    void VulkanEngine::MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
    void VulkanEngine::ShutDownEngine()
    {
        vulkanSetup->ShutDownVulkan();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

	
}
