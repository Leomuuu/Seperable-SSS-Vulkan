#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

namespace VlkEngine {
    class VulkanEngine {
    public:
        void Run(); 

        VulkanEngine(int w, int h, std::string name);
        ~VulkanEngine();

    private:
        GLFWwindow* window;
        std::string windowName;
        const uint32_t width;
        const uint32_t height;

        void InitWindow(); 
        void StartEngine(); 
        void MainLoop();
        void ShutDownEngine(); 

    };
}
