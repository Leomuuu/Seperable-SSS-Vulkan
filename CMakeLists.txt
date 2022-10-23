# CMakeList.txt: MEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("MEngine")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MENGINE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_INSTALL_PREFIX ${MENGINE_ROOT_DIR}/bin)
set(BINARY_ROOT_DIR ${CMAKE_INSTALL_PREFIX}/)
set(ENGINE_ROOT_DIR ${MENGINE_ROOT_DIR}/engine)
set(THIRD_PARTY_DIR ${ENGINE_ROOT_DIR}/3rdparty)

add_subdirectory(engine)

AUX_SOURCE_DIRECTORY(. DIR_SRCS)
add_executable (${PROJECT_NAME} ${DIR_SRCS})

# glfw glm
# glm
# include_directories(${THIRD_PARTY_DIR}/glm)
#glfw
# include_directories(${THIRD_PARTY_DIR}/glfw-3.3.8.bin.WIN64/include)
# link_directories(${THIRD_PARTY_DIR}/glfw-3.3.8.bin.WIN64/lib-vc2019)
target_link_libraries(${PROJECT_NAME}  glfw glm)
# vulkan SDK needed 
find_package(Vulkan REQUIRED FATAL_ERROR)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}  ${Vulkan_LIBRARIES})

# other files
target_link_libraries(${PROJECT_NAME}  runtime)

# add_dependencies(${PROJECT_NAME} Shaders)







