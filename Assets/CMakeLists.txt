cmake_minimum_required(VERSION 3.10)
project(ImmortalHLSL)

set(HLSL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/hlsl")
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIG>/Assets/Shaders/hlsl/")

set(HLSL_FILES
    color_space_nv122rgba.hlsl
    color_space_y2102rgba.hlsl
    color_space_yuvp2rgba.hlsl
    generatemipmaps_cube.hlsl
    generatemipmaps.hlsl
)

list(TRANSFORM HLSL_FILES PREPEND "${HLSL_DIR}/")

source_group("\\" FILES ${HLSL_FILES})

add_shader_library(${PROJECT_NAME} "${HLSL_FILES}")
