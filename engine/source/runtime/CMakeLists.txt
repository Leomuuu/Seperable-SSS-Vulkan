set(RUNTIME_DIR ${ENGINE_ROOT_DIR}/source/runtime)

file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")
add_library(runtime ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(runtime PUBLIC ${RUNTIME_DIR}/function)

target_link_libraries(runtime  glfw glm tinyobjloader)

find_package(Vulkan REQUIRED FATAL_ERROR)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(runtime ${Vulkan_LIBRARIES})