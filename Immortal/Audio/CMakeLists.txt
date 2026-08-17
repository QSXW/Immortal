cmake_minimum_required(VERSION 3.16)

set (CMAKE_VERBOSE_MAKEFILE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("ImmortalAudio" LANGUAGES CXX)

option(IMMORTAL_AUDIO_USE_SDL2 "Use SDL2 audio (callback timing like ffplay). Requires SDL2 from vcpkg." OFF)

set(AUDIO_FILES
    Device.cpp
    Device.h
    IAudioDevice.cpp
    IAudioDevice.h
    AudioStream.cpp
    AudioStream.h)

if(IMMORTAL_AUDIO_USE_SDL2)
    find_package(SDL2 CONFIG REQUIRED)
    list(APPEND AUDIO_FILES
        SDLDevice.cpp
        SDLDevice.h)
elseif(WIN32)
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
    ImmortalGraphics)

if(IMMORTAL_AUDIO_USE_SDL2)
    target_compile_definitions(${PROJECT_NAME} PRIVATE IMMORTAL_AUDIO_USE_SDL2=1)
    target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2)
endif()

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
