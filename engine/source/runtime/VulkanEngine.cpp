#include "VulkanEngine.h"
namespace VlkEngine {
    VulkanEngine::VulkanEngine(int w, int h, std::string name):width(w),height(h),windowName(name)
    {
        InitWindow();
    }
    VulkanEngine::~VulkanEngine()
    {
        ShutDownEngine();
    }
    void VulkanEngine::Run()
    {
        StartEngine();
        MainLoop();
        ShutDownEngine();
    }
    void VulkanEngine::InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    }
    void VulkanEngine::StartEngine()
    {

    }
    void VulkanEngine::MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
    void VulkanEngine::ShutDownEngine()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

	
}
