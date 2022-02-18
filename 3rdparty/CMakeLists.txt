cmake_minimum_required(VERSION 3.20)

project(3rdparty LANGUAGES CXX)
set(LIB_NOT_FOUND 0)
set(HAVE_OPENCV ${LIB_NOT_FOUND})

message("Compiler Detected: " ${CMAKE_CXX_COMPILER_ID})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(assimp_lib libassimp)
    set(opencv_lib libopencv_core453)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(assimp_lib assimp-vc142-mt)
    set(opencv_lib opencv_world455)
endif()

message("Build Type: " ${CMAKE_BUILD_TYPE})
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    string(APPEND assimp_lib "d")
    string(APPEND opencv_lib "d")

endif()

# assimp
MESSAGE("Assimp: " ${assimp_lib})
add_library(assimp INTERFACE)
set(ASSIMP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assimp)
set(ASSIMP_LIB_DIR ${ASSIMP_DIR}/bin/${CMAKE_BUILD_TYPE})
set(ENV{ASSIMP_LIB_DIR} ${ASSIMP_LIB_DIR})
target_sources(assimp INTERFACE ${ASSIMP_DIR}/include/assimp/scene.h)
target_include_directories(assimp INTERFACE ${ASSIMP_DIR}/include)
target_link_directories(assimp INTERFACE ${ASSIMP_LIB_DIR})
message("Assimp path: "  ${ASSIMP_LIB_DIR})
target_link_libraries(assimp INTERFACE ${assimp_lib})
set(ASSIMP_SHARED ${ASSIMP_LIB_DIR}/${assimp_lib}.dll PARENT_SCOPE)

# glm
add_library(glm INTERFACE)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_sources(glm INTERFACE ${GLM_DIR}/glm/glm.hpp)
target_include_directories(glm INTERFACE ${GLM_DIR})

target_compile_definitions(glm INTERFACE
    GLM_FORCE_SWIZZLE
    GLM_FORCE_RADIANS
    GLM_FORCE_CTOR_INIT
    GLM_ENABLE_EXPERIMENTAL
)
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message("Detect using a non MSVC Compiler => using --std=c++14 for glm")
    # target_compile_options(glm INTERFACE -std=c++17)
    target_compile_definitions(glm INTERFACE GLM_FORCE_CXX17)
endif()

# spdlog
add_subdirectory(spdlog)

# glfw
add_subdirectory(GLFW)

# glad
add_library(glad INTERFACE)
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Glad)
target_sources(glad INTERFACE
    ${GLAD_DIR}/include/glad/glad.h
    ${GLAD_DIR}/include/KHR/khrplatform.h
    ${GLAD_DIR}/src/glad.c)
target_include_directories(glad INTERFACE ${GLAD_DIR}/include)

# vulkan
add_library(vulkan INTERFACE)
set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
target_sources(vulkan INTERFACE ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h)
target_include_directories(vulkan INTERFACE ${VULKAN_INCLUDE_DIR})

target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

if(ANDROID)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    # Choose WSI based on VKB_WSI_SELECTION
    if (VKB_WSI_SELECTION STREQUAL XCB OR VKB_WSI_SELECTION STREQUAL XLIB OR VKB_WSI_SELECTION STREQUAL WAYLAND)
        find_package(PkgConfig REQUIRED)
    endif()
    if (VKB_WSI_SELECTION STREQUAL XCB)
        pkg_check_modules(XCB xcb REQUIRED)
        if (XCB_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
        endif()
    elseif (VKB_WSI_SELECTION STREQUAL XLIB)
        pkg_check_modules(X11 x11 REQUIRED)
        if (X11_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XLIB_KHR)
        endif()
    elseif (VKB_WSI_SELECTION STREQUAL WAYLAND)
        pkg_check_modules(WAYLAND wayland-client REQUIRED)
        if (WAYLAND_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
        endif()
    elseif (VKB_WSI_SELECTION STREQUAL D2D)
        set(DIRECT_TO_DISPLAY TRUE)
        set(DIRECT_TO_DISPLAY TRUE PARENT_SCOPE)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_DISPLAY_KHR)
    else()
        message(FATAL_ERROR "Unknown WSI")
    endif()
endif()

# imgui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_FILES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imconfig.h"
    "${IMGUI_DIR}/imgui.h"
    "${IMGUI_DIR}/imgui_internal.h"
    "${IMGUI_DIR}/imstb_rectpack.h"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imstb_textedit.h"
    "${IMGUI_DIR}/imstb_truetype.h")

add_library(imgui STATIC ${IMGUI_FILES})

target_include_directories(imgui PUBLIC ${IMGUI_DIR})

set_property(TARGET imgui PROPERTY FOLDER "3rdparty")

#glslang
option(ENABLE_SPVREMAPPER OFF)
option(ENABLE_GLSLANG_BINARIES ON)
option(ENABLE_HLSL OFF)
option(ENABLE_OPT OFF)
option(BUILD_TESTING OFF)
option(BUILD_EXTERNAL OFF)

add_subdirectory(glslang)

if (NOT TARGET glslang-default-resource-limits)
    add_library(glslang-default-resource-limits
                glslang/StandAlone/ResourceLimits.cpp)

    set_property(TARGET glslang-default-resource-limits PROPERTY FOLDER "3rdparty")

    target_include_directories(glslang-default-resource-limits
                                PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glslang/StandAlone)
endif()

if(NOT MSVC)
    target_compile_options(glslang PRIVATE "-Wno-logical-op-parentheses" "-Wno-unused-parameter")

    target_compile_options(SPIRV PRIVATE "-Wno-logical-op-parentheses" "-Wno-unused-parameter")
endif()

set_property(TARGET glslang PROPERTY FOLDER "3rdparty")
set_property(TARGET OGLCompiler PROPERTY FOLDER "3rdparty")
set_property(TARGET OSDependent PROPERTY FOLDER "3rdparty")
set_property(TARGET SPIRV PROPERTY FOLDER "3rdparty")

# SPIRV-Cross
add_subdirectory(SPIRV-Cross)

# opencv
if (WIN32)
    set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/3rdparty/opencv")
    set(OPENCV_BIN_DIR "${OpenCV_DIR}/x64/vc15/bin")
endif(WIN32)
find_package(OpenCV 4.5.5 REQUIRED)
if (OpenCV_FOUND)
    set(HAVE_OPENCV ${OpenCV_VERSION_MAJOR})
    set(OPENCV_SHARED ${OPENCV_BIN_DIR}/${opencv_lib}.dll PARENT_SCOPE)
endif(OpenCV_FOUND)

add_library("external" INTERFACE)

target_link_libraries("external" INTERFACE
    glm glad glfw glslang
    assimp vulkan imgui
    spdlog::spdlog opencv_world
    SPIRV spirv-cross-glsl
    glslang-default-resource-limits)

configure_file(Config.h.in ${CMAKE_SOURCE_DIR}/Immortal/Config.h)
