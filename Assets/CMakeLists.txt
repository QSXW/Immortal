cmake_minimum_required(VERSION 3.16)

project("Shaders")

set(GLSL_FILES
    Basic.frag
    Basic.vert
    Basic3D.frag
    Basic3D.vert
    ColorMixing.comp
    Outline.frag
    Outline.vert
    PhysicalBasedRendering.frag
    PhysicalBasedRendering.vert
    Render2D.frag
    Render2D.vert
    SimpleBlur.comp
    Texture.frag
    Texture.vert)

set(HLSL_FILES
    Basic.hlsl
    Basic3D.hlsl
    Render2D.hlsl
    Texture.hlsl)

list(TRANSFORM GLSL_FILES PREPEND "Shaders/glsl/")
list(TRANSFORM HLSL_FILES PREPEND "Shaders/hlsl/")

source_group("glsl\\" FILES ${GLSL_FILES})
source_group("hlsl\\" FILES ${HLSL_FILES})

add_custom_target(${PROJECT_NAME}
    SOURCES
        ${GLSL_FILES}
        ${HLSL_FILES}
    )
