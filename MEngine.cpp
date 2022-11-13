#include "engine/source/runtime/VulkanEngine.h"


int main() {

    VlkEngine::VulkanEngine vEngine(1600,900,"MEngine");

    try {
        vEngine.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
