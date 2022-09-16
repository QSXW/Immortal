cmake_minimum_required(VERSION 3.16)

set ( CMAKE_VERBOSE_MAKEFILE on  )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

project("Immortal Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(WORKSPACE ${CMAKE_CURRENT_SOURCE_DIR})
set(IMMORTAL_RUMTIME_DIR ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/cmake)
include(utils)

add_compile_definitions($<$<CONFIG:DEBUG>:_DEBUG>)

add_subdirectory(3rdparty)
set(IMMORTAL_ASSET_DIR "${WORKSPACE}//Assets" PARENT_SCOPE)
set(IMMORTAL_RUNTIME ${ASSIMP_SHARED} ${OPENCV_SHARED} ${MONO_SHARED} ${DAV1D_SHARED} ${FFMPEG_SHARED} PARENT_SCOPE)

add_subdirectory(Immortal)
add_subdirectory(Assets)
add_subdirectory(Samples/CMakeTest)
add_subdirectory(Samples/D3D12Sample)
add_subdirectory(Samples/VulkanSample)
add_subdirectory(Samples/OpenGLSample)
add_subdirectory(Samples/MediaTest)
add_subdirectory(Samples/ScriptBenchmark)
add_subdirectory(Samples/RawExtractor)
