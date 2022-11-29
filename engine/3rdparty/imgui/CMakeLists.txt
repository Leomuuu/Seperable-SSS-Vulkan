set(EDITOR_DIR ${ENGINE_ROOT_DIR}/source/editor)

file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

add_library(imgui ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(imgui  glfw glm)

find_package(Vulkan REQUIRED FATAL_ERROR)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(imgui ${Vulkan_LIBRARIES})
