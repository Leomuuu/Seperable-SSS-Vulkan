file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")
add_library(runtime ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(runtime  glfw glm)

find_package(Vulkan REQUIRED FATAL_ERROR)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(runtime ${Vulkan_LIBRARIES})