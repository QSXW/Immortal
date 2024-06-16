cmake_minimum_required(VERSION 3.16)

project(3rdparty LANGUAGES CXX)

set(LIB_FOUND     1)
set(LIB_NOT_FOUND 0)
set(HAVE_ASSIMP    ${LIB_NOT_FOUND})
set(HAVE_FFMPEG    ${LIB_NOT_FOUND})
set(HAVE_OPENCV    ${LIB_NOT_FOUND})
set(HAVE_DAV1D     ${LIB_NOT_FOUND})
set(HAVE_MFX       ${LIB_NOT_FOUND})
set(HAVE_MONO      ${LIB_NOT_FOUND})
set(HAVE_FREETYPE  ${LIB_NOT_FOUND})
set(HAVE_TURBOJPEG ${LIB_NOT_FOUND})
set(HAVE_NVJPEG    ${LIB_NOT_FOUND})
set(3RDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include(CheckIncludeFileCXX)

set(HAVE_MSVC false)
message(STATUS "Compiler Detected: " ${CMAKE_CXX_COMPILER_ID})
if (CMAKE_CXX_SIMULATE_ID)
    message(STATUS "Simulation Compiler Detected: " ${CMAKE_CXX_SIMULATE_ID})
    if (${CMAKE_CXX_SIMULATE_ID} STREQUAL "MSVC")
        set(HAVE_MSVC true)
    endif()
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(HAVE_MSVC true)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(ASSIMP_LIB libassimp)
    set(OPENCV_LIB libopencv_core)
elseif(HAVE_MSVC)
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
if (WIN32)
    set(SPDLOG_USE_STD_FORMAT ON)
endif()
add_subdirectory(spdlog)
set_property(TARGET spdlog PROPERTY FOLDER "3rdparty")

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
add_library(Vulkan-Headers ALIAS vulkan)

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
    find_package(PkgConfig REQUIRED)
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
        pkg_check_modules(XCB xcb)
        if (XCB_FOUND)
            target_compile_definitions(vulkan INTERFACE IMMORTAL_VK_USE_PLATFORM_XCB_KHR)
            message(STATUS "Enable VK_USE_PLATFORM_XCB_KHR")
        endif()
        pkg_check_modules(X11 x11)
        if (X11_FOUND)
            target_compile_definitions(vulkan INTERFACE IMMORTAL_VK_USE_PLATFORM_XLIB_KHR)
            message(STATUS "Enable VK_USE_PLATFORM_XLIB_KHR")
        endif()
        pkg_check_modules(WAYLAND wayland-client)
        if (WAYLAND_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
            message(STATUS "Enable VK_USE_PLATFORM_WAYLAND_KHR")
        endif()
        if (NOT (XCB_FOUND OR X11_FOUND OR WAYLAND_FOUND))
            message("Unknown WSI")
        endif()
    endif()
endif()

# glfw
if (WAYLAND_FOUND)
    message(STATUS "Enable GLFW_BUILD_WAYLAND")
    set(GLFW_BUILD_WAYLAND ON)
endif()
add_subdirectory(GLFW)
set_property(TARGET glfw PROPERTY FOLDER "3rdparty")

set(FREETYPE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/freetype" CACHE STRING "Path to freetype directory (default=${CMAKE_CURRENT_SOURCE_DIR}/freetype)")
set(FREETYPE_INCLUDE_DIR "${FREETYPE_DIR}/include")
message(STATUS "Looking for freetype directory...")
find_path(FreeType_FOUND "freetype.h" PATHS "${FREETYPE_INCLUDE_DIR}/freetype" NO_DEFAULT_PATH)
if (FreeType_FOUND)
    message(STATUS "Looking for freetype directory - found ${FREETYPE_DIR}")
    message(STATUS "Looking for freetype include - found ${FREETYPE_INCLUDE_DIR}")
    set(HAVE_FREETYPE ${LIB_FOUND})
    add_subdirectory(freetype)
    set_property(TARGET freetype PROPERTY FOLDER "3rdparty")
endif()

set(LIBJPEGTURBO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo" CACHE STRING "Path to libjpeg-turbo directory (default=${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo)")
set(LIBJPEGTURBO_INCLUDE_DIR "${LIBJPEGTURBO_DIR}/include")
message(STATUS "Looking for freetype directory...")
find_path(LIBJPEGTURBO_FOUND "turbojpeg.h" PATHS "${LIBJPEGTURBO_INCLUDE_DIR}" NO_DEFAULT_PATH)
if (LIBJPEGTURBO_FOUND)
    message(STATUS "Looking for libjpeg-turbo directory - found ${LIBJPEGTURBO_DIR}")
    message(STATUS "Looking for libjpeg-turbo include   - found ${LIBJPEGTURBO_INCLUDE_DIR}")
    set(HAVE_TURBOJPEG ${LIB_FOUND})
    add_library(turbojpeg INTERFACE)
    target_include_directories(turbojpeg INTERFACE ${LIBJPEGTURBO_INCLUDE_DIR})
    target_link_libraries(turbojpeg INTERFACE "${LIBJPEGTURBO_DIR}/lib/turbojpeg-static.lib")
    set_property(TARGET turbojpeg PROPERTY FOLDER "3rdparty")
endif()

set(CUDA_SDK_PATH $ENV{CUDA_PATH})
if (CUDA_SDK_PATH)
    message(STATUS "Found CUDA - ${CUDA_SDK_PATH}")
    set(CUDA_SDK_INCLUDE_DIR "${CUDA_SDK_PATH}/include")
    message(STATUS "Looking for nvjpeg directory...")
    find_path(NVJPEG_FOUND "nvjpeg.h" PATHS "${CUDA_SDK_INCLUDE_DIR}" NO_DEFAULT_PATH)
    if (NVJPEG_FOUND)
        message(STATUS "Looking for nvJpeg directory - found ${CUDA_SDK_INCLUDE_DIR}")
        set(HAVE_NVJPEG ${LIB_FOUND})
        add_library(nvJPEG INTERFACE)
        target_include_directories(nvJPEG INTERFACE ${CUDA_SDK_INCLUDE_DIR})
        set(CUDA_SDK_LIB_DIR "${CUDA_SDK_PATH}/lib/x64")
        target_link_libraries(nvJPEG INTERFACE
            "${CUDA_SDK_LIB_DIR}/nvjpeg.lib"
            "${CUDA_SDK_LIB_DIR}/cudart.lib")
        set_property(TARGET nvJPEG PROPERTY FOLDER "3rdparty")
    endif()
endif()

# imgui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_FILES
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imconfig.h
    imgui.h
    imgui_internal.h
    imstb_rectpack.h
    imgui_tables.cpp
    imstb_textedit.h
    imstb_truetype.h
    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_glfw.cpp
    $<$<PLATFORM_ID:Windows>:backends/imgui_impl_win32.cpp>)

list(TRANSFORM IMGUI_FILES PREPEND "${IMGUI_DIR}/")

add_library(imgui STATIC
    ${IMGUI_FILES}
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/ImGuizmo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/ImGuizmo.h")

if (HAVE_FREETYPE)
    target_compile_definitions(imgui PRIVATE "IMGUI_ENABLE_FREETYPE")
    target_sources(imgui PRIVATE
        ${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp
        ${IMGUI_DIR}/misc/freetype/imgui_freetype.h)
    target_link_libraries(imgui PRIVATE freetype)
endif()

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo")

target_link_libraries("imgui" PUBLIC glfw)

set_property(TARGET imgui PROPERTY FOLDER "3rdparty")

#glslang
option(ENABLE_SPVREMAPPER OFF)
option(ENABLE_GLSLANG_BINARIES OFF)
option(ENABLE_HLSL OFF)
option(ENABLE_OPT OFF)
option(BUILD_TESTING OFF)
option(BUILD_EXTERNAL OFF)

add_subdirectory(glslang)

if (NOT TARGET glslang-default-resource-limits)
    add_library(glslang-default-resource-limits
                glslang/StandAlone/ResourceLimits.cpp)

    set_property(TARGET glslang-default-resource-limits PROPERTY FOLDER "3rdparty")

    target_include_directories(glslang-default-resource-limits PUBLIC
                                ${CMAKE_CURRENT_SOURCE_DIR}/glslang/
                                ${CMAKE_CURRENT_SOURCE_DIR}/glslang/glslang)
endif()

if(NOT MSVC)
    target_compile_options(glslang PRIVATE "-Wno-logical-op-parentheses" "-Wno-unused-parameter")

    target_compile_options(SPIRV PRIVATE "-Wno-logical-op-parentheses" "-Wno-unused-parameter")
endif()

set_property(TARGET glslang     PROPERTY FOLDER "3rdparty")
set_property(TARGET OSDependent PROPERTY FOLDER "3rdparty")
set_property(TARGET SPIRV       PROPERTY FOLDER "3rdparty")

# SPIRV-Cross
add_subdirectory(SPIRV-Cross)

set_property(TARGET spirv-cross         PROPERTY FOLDER "3rdparty")
set_property(TARGET spirv-cross-c       PROPERTY FOLDER "3rdparty")
set_property(TARGET spirv-cross-core    PROPERTY FOLDER "3rdparty")
set_property(TARGET spirv-cross-cpp     PROPERTY FOLDER "3rdparty")
set_property(TARGET spirv-cross-glsl    PROPERTY FOLDER "3rdparty")
set_property(TARGET spirv-cross-hlsl    PROPERTY FOLDER "3rdparty")
set_property(TARGET spirv-cross-msl     PROPERTY FOLDER "3rdparty")
set_property(TARGET spirv-cross-reflect PROPERTY FOLDER "3rdparty")
set_property(TARGET spirv-cross-util    PROPERTY FOLDER "3rdparty")

set(SHARED_SUFFIX "")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SHARED_SUFFIX "d")
endif()

# assimp
set(Assimp_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assimp" CACHE STRING "Path to Assimp directory (default=${CMAKE_CURRENT_SOURCE_DIR}/Assimp)")
message(STATUS "Looking for Assimp directory...")
find_package(Assimp QUIET PATHS ${Assimp_DIR})
if (Assimp_FOUND)
    message(STATUS "Looking for Assimp directory - found ${ASSIMP_ROOT_DIR}")
    set(ASSIMP_BIN_DIR "${ASSIMP_ROOT_DIR}/bin")
    set(HAVE_ASSIMP "${Assimp_VERSION_MAJOR}")
    if (HAVE_MSVC)
        set(LOCAL_ASSIMP_SHARED "${ASSIMP_BIN_DIR}/${ASSIMP_LIB}${SHARED_SUFFIX}.dll")
    else()
        set(LOCAL_ASSIMP_SHARED "${ASSIMP_BIN_DIR}/libassimp${SHARED_SUFFIX}-${Assimp_VERSION_MAJOR}.dll")
    endif()
    message(STATUS "Looking for Assimp library ${LOCAL_ASSIMP_SHARED}")
    set(ASSIMP_SHARED ${LOCAL_ASSIMP_SHARED} PARENT_SCOPE)
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
    set(OPENCV_LIB_SUFFIX "${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}${SHARED_SUFFIX}.dll")
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
set(MFX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libmfx" CACHE STRING "Path to Mono directory (default=${CMAKE_CURRENT_SOURCE_DIR}/libmfx)")
set(MFX_LIBRARY_DIR "${MFX_DIR}/lib")
set(MFX_INCLUDE_DIR "${MFX_DIR}/include")
message(STATUS "Looking for MFX directory...")
find_path(MFX_FOUND "mfx/mfxadapter.h" PATHS "${MFX_INCLUDE_DIR}" NO_DEFAULT_PATH)
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
set(FFMPEG_LIBS avcodec avdevice avformat avutil swresample)
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

set_property(TARGET raw   PROPERTY FOLDER "3rdparty")
set_property(TARGET raw_r PROPERTY FOLDER "3rdparty")

# LightWSL
add_subdirectory(LightWSI)

add_subdirectory(DirectXShaderCompilerHeaders)
set(DXC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/DirectXShaderCompilerHeaders/dxc")
if (WIN32)
    set(DXC_SHARED_DIR "${DXC_DIR}/bin/Windows/x64")
    set(DXC_SHARED
        "${DXC_SHARED_DIR}/dxcompiler.dll"
        "${DXC_SHARED_DIR}/dxil.dll"
        PARENT_SCOPE)
elseif(APPLE)

elseif(UNIX)
set(DXC_SHARED_DIR "${DXC_DIR}/lib/Linux")
    set(DXC_SHARED
        "${DXC_SHARED_DIR}/libdxcompiler.so"
        "${DXC_SHARED_DIR}/libdxil.so"
        PARENT_SCOPE)
endif()

if (APPLE)
    add_subdirectory(metal-cpp)
endif()

add_subdirectory(VulkanMemoryAllocator)

add_subdirectory(concurrentqueue)

add_library("external" INTERFACE)

target_link_libraries("external" INTERFACE
    glm glad glfw glslang glslang-default-resource-limits
    Vulkan-Headers imgui spdlog_header_only
    SPIRV spirv-cross-glsl
    $<$<BOOL:${Assimp_FOUND}>:assimp::assimp>
    $<$<BOOL:${MONO_FOUND}>:mono>
)

add_library("ExternalVision" INTERFACE)
target_link_libraries("ExternalVision" INTERFACE
    libraw::libraw
    $<$<BOOL:${DAV1D_FOUND}>:dav1d>
    $<$<BOOL:${MFX_FOUND}>:intel_mfx>
    $<$<BOOL:${OpenCV_FOUND}>:opencv_world>
    $<$<BOOL:${FFmpeg_FOUND}>:FFmpeg>
    $<$<BOOL:${LIBJPEGTURBO_FOUND}>:turbojpeg>
    $<$<BOOL:${NVJPEG_FOUND}>:nvJPEG>)

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Immortal 3rdparty configuration")
message(STATUS)
if (Assimp_FOUND)
    message(STATUS "Immortal will be compiled with Assimp        ------ Yes")
else()
    message(STATUS "Immortal will be compiled with Assimp        ------ No")
endif()
if (DAV1D_FOUND)
    message(STATUS "Immortal will be compiled with Dav1d         ------ Yes")
else()
    message(STATUS "Immortal will be compiled with Dav1d         ------ No")
endif()
if (MFX_FOUND)
    message(STATUS "Immortal will be compiled with MFX           ------ Yes")
else()
    message(STATUS "Immortal will be compiled with MFX           ------ No")
endif()
if (MONO_FOUND)
    message(STATUS "Immortal will be compiled with Mono          ------ Yes")
else()
    message(STATUS "Immortal will be compiled with Mono          ------ No")
endif()
if (OpenCV_FOUND)
    message(STATUS "Immortal will be compiled with OpenCV        ------ Yes")
else()
    message(STATUS "Immortal will be compiled with OpenCV        ------ No")
endif()
if (FFmpeg_FOUND)
    message(STATUS "Immortal will be compiled with FFmpeg        ------ Yes")
    else()
    message(STATUS "Immortal will be compiled with FFmpeg        ------ No")
endif()
if (FreeType_FOUND)
    message(STATUS "Immortal will be compiled with freetype      ------ Yes")
else()
    message(STATUS "Immortal will be compiled with freetype      ------ No")
endif()
if (LIBJPEGTURBO_FOUND)
    message(STATUS "Immortal will be compiled with libjpeg-turbo ------ Yes")
else()
    message(STATUS "Immortal will be compiled with libjpeg-turbo ------ No")
endif()
if (NVJPEG_FOUND)
    message(STATUS "Immortal will be compiled with nvJPEG        ------ Yes")
else()
    message(STATUS "Immortal will be compiled with nvJPEG        ------ No")
endif()
message(STATUS "----------------------------------------------------------------------------------")

configure_file(Config.h.in ${WORKSPACE}/Immortal/Config.h)
