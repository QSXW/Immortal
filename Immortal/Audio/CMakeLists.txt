cmake_minimum_required(VERSION 3.16)

set (CMAKE_VERBOSE_MAKEFILE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("ImmortalAudio" LANGUAGES CXX)

set(AUDIO_FILES
    IAudioDevice.cpp
    IAudioDevice.h
    Device.cpp
    Device.h)

if (WIN32)
    list(APPEND AUDIO_FILES
        WASAPI.cpp
        WASAPI.h)
elseif(APPLE)
    list(APPEND AUDIO_FILES
        CoreAudio.mm
        CoreAudio.h)
elseif(UNIX)
    list(APPEND AUDIO_FILES
        ALSA.cpp
        ALSA.h)
endif()

list(TRANSFORM AUDIO_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

source_group("\\" FILES ${AUDIO_FILES})

add_library(${PROJECT_NAME} STATIC ${AUDIO_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC 
    ImmortalShared
    ImmortalVision)

if (NOT APPLE)
    target_precompile_headers(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/AudioPrecompiledHeader.h>)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreAudio"
        "-framework AudioToolBox")
endif()
