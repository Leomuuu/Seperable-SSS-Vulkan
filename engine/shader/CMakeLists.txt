# shaders
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/ 
  $ENV{VULKAN_SDK}/Bin32/
)

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${ENGINE_ROOT_DIR}/shader/*.frag"
  "${ENGINE_ROOT_DIR}/shader/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${ENGINE_ROOT_DIR}/shader/${FILE_NAME}.spv")
  message("BUILD SHADERS")
  execute_process(
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
  )
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

