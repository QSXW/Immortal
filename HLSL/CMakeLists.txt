cmake_minimum_required(VERSION 3.10)
project(ImmortalHLSL)

set(HLSL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")

set(COMPUTE_HLSL_FILES
    color_space_nv122rgba.hlsl
    color_space_y2102rgba.hlsl
    color_space_yuvp2rgba.hlsl
    generatemipmaps_cube.hlsl
    generatemipmaps.hlsl
    ibl_irradiance.hlsl
    brdf_lut.hlsl
)

set(GRAPHICS_HLSL_FILES
    atmosphere.hlsl
    deferred_resolve.hlsl
    deferred_resolve_pbr.hlsl
)

list(TRANSFORM COMPUTE_HLSL_FILES PREPEND "${HLSL_DIR}/")
list(TRANSFORM GRAPHICS_HLSL_FILES PREPEND "${HLSL_DIR}/")

source_group("\\" FILES ${COMPUTE_HLSL_FILES} ${GRAPHICS_HLSL_FILES})

# Match copy target: multi-config uses $<CONFIG>; single-config uses CMAKE_BUILD_TYPE (Ninja has empty $<CONFIG>).
if(CMAKE_CONFIGURATION_TYPES)
    set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIG>/Assets/Shaders/hlsl")
else()
    set(_shader_bt "${CMAKE_BUILD_TYPE}")
    if(NOT _shader_bt)
        set(_shader_bt "Debug")
    endif()
    set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_shader_bt}/Assets/Shaders/hlsl")
endif()

add_shader_library(${PROJECT_NAME}
    HLSL_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_DIR "${SHADER_OUTPUT_DIR}"
    COMPUTE "${COMPUTE_HLSL_FILES}"
    GRAPHICS "${GRAPHICS_HLSL_FILES}"
)
