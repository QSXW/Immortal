cmake_minimum_required(VERSION 3.16)

set ( CMAKE_VERBOSE_MAKEFILE on  )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

project("Immortal Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(WORKSPACE ${CMAKE_CURRENT_SOURCE_DIR})
set(IMMORTAL_RUMTIME_DIR ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/cmake)
include(utils)

add_compile_definitions(
    "_UNICODE"
    "UNICODE")

if (UNIX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions("_DEBUG")
endif()

add_subdirectory(3rdparty)
set(IMMORTAL_ASSET_DIR "${WORKSPACE}//Assets")
set(IMMORTAL_RUNTIME ${ASSIMP_SHARED} ${OPENCV_SHARED} ${MONO_SHARED} ${DAV1D_SHARED} ${FFMPEG_SHARED})

add_subdirectory(Immortal)
add_subdirectory(Assets)
add_subdirectory(Samples/CMakeTest)
add_subdirectory(Samples/ImmortalEditor)
add_subdirectory(Samples/MediaTest)
add_subdirectory(Samples/ScriptBenchmark)
add_subdirectory(Samples/RawExtractor)
add_subdirectory(Samples/ImGuiExample)
add_subdirectory(Samples/HelloTriangle)

set(IMMORTAL_ASSET_DIR ${IMMORTAL_ASSET_DIR} PARENT_SCOPE)
set(IMMORTAL_RUNTIME ${IMMORTAL_RUNTIME} PARENT_SCOPE)
