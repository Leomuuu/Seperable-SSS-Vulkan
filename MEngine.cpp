#include "engine/source/runtime/VulkanEngine.h"
#include "engine/source/editor/Editor.h"


int main() {

    VlkEngine::VulkanEngine vEngine(1600,900,"MEngine");
    VlkEngine::Editor editor(&vEngine);

    try {
        editor.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
