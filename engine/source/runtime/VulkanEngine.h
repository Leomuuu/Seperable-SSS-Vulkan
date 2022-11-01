#pragma once

#include "function/render/VulkanSetup.h"
#include "function/render/RenderPipline.h"
#include "function/render/RenderBuffer.h"

namespace VlkEngine {
    class VulkanEngine {
    public:
        void Run(); 

        VulkanEngine(int w, int h, std::string name);
        ~VulkanEngine();

    private:        
        void InitEngine(); 
        void StartEngine(); 
        void MainLoop();
        void ShutDownEngine();

        void DrawFrame();
        
        // window
        GLFWwindow* window;
        std::string windowName;
        const uint32_t width;
        const uint32_t height;

        // render
        VulkanSetup* vulkanSetup;
        RenderPipline* renderPipline;
        RenderBuffer* renderBuffer;
    };
}
