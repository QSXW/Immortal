cmake_minimum_required(VERSION 3.16)

project(3rdparty LANGUAGES CXX)

set(LIB_FOUND     1)
set(LIB_NOT_FOUND 0)
set(HAVE_ASSIMP ${LIB_NOT_FOUND})
set(HAVE_FFMPEG ${LIB_NOT_FOUND})
set(HAVE_OPENCV ${LIB_NOT_FOUND})
set(HAVE_DAV1D  ${LIB_NOT_FOUND})
set(HAVE_MFX    ${LIB_NOT_FOUND})
set(HAVE_MONO   ${LIB_NOT_FOUND})

include(CheckIncludeFileCXX)

message("Compiler Detected: " ${CMAKE_CXX_COMPILER_ID})
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(ASSIMP_LIB libassimp)
    set(OPENCV_LIB libopencv_core)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(ASSIMP_LIB assimp-vc142-mt)
    set(OPENCV_LIB opencv_world)
endif()

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

target_compile_definitions(vulkan INTERFACE
    "VK_NO_PROTOTYPES"
    "VK_ENABLE_BETA_EXTENSIONS")

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
        message("Unknown WSI")
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

set(SHARED_SUFFIX .dll)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    string(PREPEND SHARED_SUFFIX d)
endif()

# assimp
set(Assimp_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assimp" CACHE STRING "Path to Assimp directory (default=${CMAKE_CURRENT_SOURCE_DIR}/Assimp)")
message(STATUS "Looking for Assimp directory...")
find_package(Assimp QUIET PATHS ${Assimp_DIR})
if (Assimp_FOUND)
    message(STATUS "Looking for Assimp directory - found ${ASSIMP_ROOT_DIR}")
    set(ASSIMP_BIN_DIR "${ASSIMP_ROOT_DIR}/bin")
    set(HAVE_ASSIMP "${Assimp_VERSION_MAJOR}")
    set(ASSIMP_SHARED "${ASSIMP_BIN_DIR}/${ASSIMP_LIB}${SHARED_SUFFIX}" PARENT_SCOPE)
else()
    message(STATUS "Looking for Assimp directory - not found")
endif(Assimp_FOUND)

# opencv
find_package(OpenCV QUIET)
if (OpenCV_FOUND)
    set(HAVE_OPENCV "${OpenCV_VERSION_MAJOR}")
    if (WIN32)
        set(OPENCV_BIN_DIR "${OpenCV_DIR}/x64/${OpenCV_RUNTIME}/bin")
    endif(WIN32)
    set(OPENCV_LIB_SUFFIX "${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}${SHARED_SUFFIX}")
    list(TRANSFORM OPENCV_LIB PREPEND "${OPENCV_BIN_DIR}/")
    list(TRANSFORM OPENCV_LIB APPEND "${OPENCV_LIB_SUFFIX}")
    set(OPENCV_SHARED "${OPENCV_LIB}" PARENT_SCOPE)
endif(OpenCV_FOUND)

# dav1d
set(DAV1D_COMMON_HEADER "dav1d/dav1d.h")
if (WIN32)
    set(DAV1D_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dav1d")
    set(DAV1D_BINARY_DIR "${DAV1D_DIR}/bin")
    set(DAV1D_LIBRARY_DIR "${DAV1D_DIR}/lib")
    set(DAV1D_INCLUDE_DIR "${DAV1D_DIR}/include")
endif(WIN32)

list(APPEND CMAKE_REQUIRED_INCLUDES ${DAV1D_INCLUDE_DIR})
CHECK_INCLUDE_FILE_CXX(${DAV1D_COMMON_HEADER} DAV1D_FOUND)
if (DAV1D_FOUND)
    set(HAVE_DAV1D ${LIB_FOUND})
    add_library("dav1d" INTERFACE)
    target_include_directories("dav1d" INTERFACE ${DAV1D_INCLUDE_DIR})
    target_link_directories("dav1d" INTERFACE ${DAV1D_LIBRARY_DIR})
    target_link_libraries("dav1d" INTERFACE libdav1d)
    set(DAV1D_SHARED
        "${DAV1D_BINARY_DIR}/dav1d.dll" PARENT_SCOPE)
endif()

# Intel mfx
set(MFX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libmfx")
set(MFX_LIBRARY_DIR "${MFX_DIR}/lib")
set(MFX_INCLUDE_DIR "${MFX_DIR}/include")
find_path(MFX_FOUND "libmfx" ${MFX_DIR})
if (MFX_FOUND)
    set(HAVE_MFX ${LIB_FOUND})
    add_library("intel_mfx" INTERFACE)
    target_include_directories("intel_mfx" INTERFACE ${MFX_INCLUDE_DIR})
    target_link_directories("intel_mfx" INTERFACE ${MFX_LIBRARY_DIR})
    target_link_libraries("intel_mfx" INTERFACE libmfx)
endif()

# Mono
set(MONO_LIB "mono-2.0-sgen")
set(MONO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Mono" CACHE STRING "Path to Mono directory (default=${CMAKE_CURRENT_SOURCE_DIR}/Mono)")
set(MONO_LIBRARY_DIR "${MONO_DIR}/lib")
set(MONO_BINARY_DIR  "${MONO_DIR}/bin")
set(MONO_INCLUDE_DIR "${MONO_DIR}/include/mono-2.0")
message(STATUS "Looking for Mono directory...")
find_path(MONO_FOUND "mono-2.0-sgen.dll" PATHS "${MONO_BINARY_DIR}" NO_DEFAULT_PATH)
if (MONO_FOUND)
    message(STATUS "Looking for Mono directory - found ${MONO_DIR}")
    message(STATUS "Looking for Mono include   - found ${MONO_INCLUDE_DIR}")
    set(HAVE_MONO ${LIB_FOUND})
    add_library("mono" INTERFACE)
    target_include_directories("mono" INTERFACE ${MONO_INCLUDE_DIR})
    target_link_directories("mono" INTERFACE ${MONO_LIBRARY_DIR})
    target_link_libraries("mono" INTERFACE ${MONO_LIB})
    if (WIN32)
        set(MONO_SHARED "${MONO_BINARY_DIR}/${MONO_LIB}.dll" PARENT_SCOPE)
    endif()
else()
    message(STATUS "Looking for Mono directory - not found")
endif()

# FFmpeg
set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil)
set(FFMPEG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/FFmpeg" CACHE STRING "Path to FFmpeg directory (default=${CMAKE_CURRENT_SOURCE_DIR}/FFmpeg)")
set(FFMPEG_LIBRARY_DIR "${FFMPEG_DIR}/lib")
set(FFMPEG_BINARY_DIR  "${FFMPEG_DIR}/bin")
set(FFMPEG_INCLUDE_DIR "${FFMPEG_DIR}/include")
message(STATUS "Looking for FFmpeg directory...")
find_path(FFmpeg_FOUND "avformat.h" PATHS "${FFMPEG_INCLUDE_DIR}/libavformat" NO_DEFAULT_PATH)
if (FFmpeg_FOUND)
    message(STATUS "Looking for FFmpeg directory - found ${FFMPEG_DIR}")
    message(STATUS "Looking for FFmpeg include - found ${FFMPEG_INCLUDE_DIR}")
    set(HAVE_FFMPEG ${LIB_FOUND})
    add_library("FFmpeg" INTERFACE)
    target_include_directories("FFmpeg" INTERFACE ${FFMPEG_INCLUDE_DIR})
    target_link_directories("FFmpeg" INTERFACE ${FFMPEG_LIBRARY_DIR})
    target_link_libraries("FFmpeg" INTERFACE ${FFMPEG_LIBS})
    if (WIN32)
        set(FFMPEG_SHARED "${FFMPEG_BINARY_DIR}/" PARENT_SCOPE)
    endif()
else()
    message(STATUS "Looking for FFmpeg directory - not found")
endif()

# LibRaw
set(LIBRAW_PATH "${CMAKE_CURRENT_SOURCE_DIR}/LibRaw/" CACHE STRING "LibRaw Path" FORCE)
add_subdirectory(LibRaw-cmake)

add_library("external" INTERFACE)

target_link_libraries("external" INTERFACE
    glm glad glfw glslang glslang-default-resource-limits
    vulkan imgui spdlog::spdlog_header_only
    SPIRV spirv-cross-glsl libraw::libraw
    $<$<BOOL:${Assimp_FOUND}>:assimp::assimp>
    $<$<BOOL:${DAV1D_FOUND}>:dav1d>
    $<$<BOOL:${MFX_FOUND}>:intel_mfx>
    $<$<BOOL:${MONO_FOUND}>:mono>
    $<$<BOOL:${OpenCV_FOUND}>:opencv_world>
    $<$<BOOL:${FFmpeg_FOUND}>:FFmpeg>)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Immortal 3rdparty configuration")
message(STATUS)
if (Assimp_FOUND)
    message(STATUS "Immortal will be compiled with Assimp ------ Yes")
else()
    message(STATUS "Immortal will be compiled with Assimp ------ No")
endif()
if (DAV1D_FOUND)
    message(STATUS "Immortal will be compiled with Dav1d  ------ Yes")
else()
    message(STATUS "Immortal will be compiled with Dav1d  ------ No")
endif()
if (MFX_FOUND)
    message(STATUS "Immortal will be compiled with MFX    ------ Yes")
else()
    message(STATUS "Immortal will be compiled with MFX    ------ No")
endif()
if (MONO_FOUND)
    message(STATUS "Immortal will be compiled with Mono   ------ Yes")
else()
    message(STATUS "Immortal will be compiled with Mono   ------ No")
endif()
if (OpenCV_FOUND)
    message(STATUS "Immortal will be compiled with OpenCV ------ Yes")
else()
    message(STATUS "Immortal will be compiled with OpenCV ------ No")
endif()
if (FFmpeg_FOUND)
    message(STATUS "Immortal will be compiled with FFmpeg ------ Yes")
    else()
    message(STATUS "Immortal will be compiled with FFmpeg ------ No")
endif()
message(STATUS "----------------------------------------------------------------------------------")

configure_file(Config.h.in ${WORKSPACE}/Immortal/Config.h)
