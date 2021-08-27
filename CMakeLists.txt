cmake_minimum_required(VERSION 3.21)

set ( CMAKE_VERBOSE_MAKEFILE on  )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

project(ImmortalEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(3rdparty)
add_subdirectory(Immortal)
add_subdirectory(CmakeTest)
add_subdirectory(VulkanSample)
add_subdirectory(OpenGLSample)
add_subdirectory(Editor)
