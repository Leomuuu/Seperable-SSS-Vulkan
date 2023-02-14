#pragma once

#include "function/render/VulkanBase.h"
#include "function/render/VulkanPBR.h"
#include "function/render/VulkanShadowMap.h"
#include "function/render/VulkanSSSS.h"
#include "function/render/Camera.h"
#include "function/input/InputSystem.h"
#include "resource/ModelManager.h"
#include "resource/PbrModelManager.h"

#include <iostream>
#include <fstream>

namespace VlkEngine {
    class VulkanEngine {
        friend class Editor;
        friend class VulkanBase;
        friend class VulkanPBR;
        friend class VulkanShadowMap;
        friend class VulkanSSSS;
    public:
        void Run(); 
        
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
        static void MouseMovecallback(GLFWwindow* window, double xpos, double ypos);
        static void KeyInputcallback(GLFWwindow* window, int key, int scancode, int action, int mods);


        VulkanEngine(int w, int h, std::string name);
        ~VulkanEngine();

    private:        
        void InitEngine(); 
        void StartEngine(); 
        void MainLoop();
        void ShutDownEngine();

        void DrawFrame();

        void WindowSurfaceChange();
        
        // window
        GLFWwindow* window;
        std::string windowName;
        const uint32_t width;
        const uint32_t height;

        // render
        uint32_t currentFrame = 0;
        VulkanBase* vulkanBase;
        bool framebufferResized = false;

        // camera
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Camera* camera;

        // input
        InputSystem* inputSystem;

        // resource
        ModelManager* modelManager;

        // light
        glm::vec3 lightPosition=glm::vec3(2);
        glm::vec3 lightRadiance = glm::vec3(6);
        float lightFov = 30.0f;

    };
}
