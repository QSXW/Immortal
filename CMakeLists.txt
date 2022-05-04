cmake_minimum_required(VERSION 3.16)

set ( CMAKE_VERBOSE_MAKEFILE on  )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

project("Immortal Engine")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(WORKSPACE ${CMAKE_CURRENT_SOURCE_DIR})
set(IMMORTAL_RUMTIME_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(ASSETS_DIR "${WORKSPACE}//Assets")

add_subdirectory(3rdparty)
add_subdirectory(Immortal)
add_subdirectory(Samples/CmakeTest)
add_subdirectory(Samples/D3D12Sample)
add_subdirectory(Samples/VulkanSample)
add_subdirectory(Samples/OpenGLSample)
add_subdirectory(Samples/MediaTest)
add_subdirectory(Samples/ScriptBenchmark)
add_subdirectory(Assets)
