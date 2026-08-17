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
    ibl_prefilter.hlsl
    brdf_lut.hlsl
    depth2rgb_cs.hlsl
    equirect2cube.hlsl
    ui_present_scale.hlsl
)

set(COMPUTE_HLSL_VARIANTS
    "${HLSL_DIR}/GaussianBlur.hlsl|Blur|GaussianBlur"
    "${HLSL_DIR}/lut.hlsl|InterpolateNearest|lut_nearest"
    "${HLSL_DIR}/lut.hlsl|InterpolateTrilinear|lut_trilinear"
)
set(COMPUTE_HLSL_VARIANT_SOURCE_FILES
    "${HLSL_DIR}/GaussianBlur.hlsl"
    "${HLSL_DIR}/lut.hlsl"
)

set(GRAPHICS_HLSL_FILES
    atmosphere.hlsl
    post_hdr_present.hlsl|PSMain
    depth2rgb_present.hlsl
    gbuffer_screen_resolve.hlsl|PSMainPhong|IMMORTAL_GBUFFER_SCREEN_VARIANT=0
    gbuffer_screen_resolve.hlsl|PSMainPBR|IMMORTAL_GBUFFER_SCREEN_VARIANT=1
    gbuffer_screen_resolve.hlsl|PSMainNPR|IMMORTAL_GBUFFER_SCREEN_VARIANT=2
    editor_grid_underlay.hlsl
    editor_grid_post.hlsl
    selection_outline.hlsl
    render2d.hlsl
    skybox.hlsl
)

# Mesh + PS: "file.hlsl" (pixel entry PSMain) or "file.hlsl|PixelEntry" — see Scripts/cmake/utils.cmake add_shader_library.
set(GRAPHICS_MESH_SPEC
    meshlet_shadow_depth.hlsl
    meshlet.hlsl|PSMain
    meshlet.hlsl|PSMainPhong
    meshlet.hlsl|PSMainPBR
    meshlet.hlsl|PSMainNPR
    meshlet.hlsl|PSMainGBuffer
)

set(GRAPHICS_MESH_HLSL_FILES "")
set(GRAPHICS_MESH_SOURCE_GROUP_FILES "")
foreach(_spec ${GRAPHICS_MESH_SPEC})
    if(_spec MATCHES "^([^|]+)\\|(.+)$")
        set(_mesh_file "${CMAKE_MATCH_1}")
        list(APPEND GRAPHICS_MESH_HLSL_FILES "${HLSL_DIR}/${_mesh_file}|${CMAKE_MATCH_2}")
        list(APPEND GRAPHICS_MESH_SOURCE_GROUP_FILES "${HLSL_DIR}/${_mesh_file}")
    else()
        list(APPEND GRAPHICS_MESH_HLSL_FILES "${HLSL_DIR}/${_spec}")
        list(APPEND GRAPHICS_MESH_SOURCE_GROUP_FILES "${HLSL_DIR}/${_spec}")
    endif()
endforeach()
list(APPEND GRAPHICS_MESH_SOURCE_GROUP_FILES "${HLSL_DIR}/lighting.hlsli")
list(APPEND GRAPHICS_MESH_SOURCE_GROUP_FILES "${HLSL_DIR}/gbuffer_screen_resolve.hlsl")
list(APPEND GRAPHICS_MESH_SOURCE_GROUP_FILES "${HLSL_DIR}/post_hdr_present.hlsl")
list(REMOVE_DUPLICATES GRAPHICS_MESH_SOURCE_GROUP_FILES)

list(TRANSFORM COMPUTE_HLSL_FILES PREPEND "${HLSL_DIR}/")
list(TRANSFORM GRAPHICS_HLSL_FILES PREPEND "${HLSL_DIR}/")

source_group("\\" FILES
    ${COMPUTE_HLSL_FILES}
    ${COMPUTE_HLSL_VARIANT_SOURCE_FILES}
    ${GRAPHICS_HLSL_FILES}
    ${GRAPHICS_MESH_SOURCE_GROUP_FILES}
)

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
    COMPUTE "${COMPUTE_HLSL_FILES}" "${COMPUTE_HLSL_VARIANTS}"
    GRAPHICS "${GRAPHICS_HLSL_FILES}"
    GRAPHICS_MESH "${GRAPHICS_MESH_HLSL_FILES}"
    RUNTIME_HLSL "flip.hlsl"
)
