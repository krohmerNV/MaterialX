file(GLOB source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

target_sources(MaterialXTest PUBLIC ${source} ${headers})

target_compile_definitions(MaterialXTest PRIVATE
        MATERIALX_MDL_COMPILER_EXECUTABLE=\"${MATERIALX_MDL_COMPILER_EXECUTABLE}\"
        MATERIALX_MDL_RENDER_EXECUTABLE=\"${MATERIALX_MDL_RENDER_EXECUTABLE}\"
)

add_tests("${source}")

assign_source_group("Source Files" ${source})
assign_source_group("Header Files" ${headers})
