#include "engine/source/runtime/VulkanEngine.h"


int main() {

    VlkEngine::VulkanEngine vEngine(800,600,"Learn");

    try {
        vEngine.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
