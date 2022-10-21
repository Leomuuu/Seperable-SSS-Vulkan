aux_source_directory(. SRC_LIST) 
add_library(runtime ${SRC_LIST})

target_link_libraries(runtime  glfw glm)

find_package(Vulkan REQUIRED FATAL_ERROR)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(runtime ${Vulkan_LIBRARIES})