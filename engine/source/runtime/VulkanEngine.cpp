#include "VulkanEngine.h"
namespace VlkEngine {
    VulkanEngine::VulkanEngine(int w, int h, std::string name):width(w),height(h),windowName(name)
    {
        InitEngine();
    }
    VulkanEngine::~VulkanEngine()
    {
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

        renderSystem = new RenderSystem();
    }
    void VulkanEngine::StartEngine()
    {
        if (renderSystem != nullptr) {
            if (!renderSystem->CreateInstance()) {
                throw std::runtime_error("failed to create vulkan instance!");
            }
        }
    }
    void VulkanEngine::MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
    void VulkanEngine::ShutDownEngine()
    {
        if (renderSystem != nullptr) {
            renderSystem->DestroyInstance();
        }
        glfwDestroyWindow(window);
        glfwTerminate();
    }

	
}
