file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

set(TARGET "MaterialXRenderMdlStandalone")
add_executable(${TARGET} ${materialx_source} ${materialx_headers})

find_package(glfw3 CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(SPIRV-Tools-opt CONFIG REQUIRED)

target_link_libraries(${TARGET}
    PRIVATE
        MaterialXCore
        MaterialXFormat
        MaterialXGenShader
        MaterialXGenMdl
        mdl::mdl_sdk
        mdl::nv_openimageio
        mdl::dds
        glfw
        Vulkan::Headers
        volk::volk
        glslang::glslang
        glslang::glslang-default-resource-limits
        glslang::SPIRV
        SPIRV-Tools-opt
    )

# copy the MDL SDK binaries to the output folder as the easiest solution without changing the PATH for execution
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
    COMMAND_EXPAND_LISTS
    )
