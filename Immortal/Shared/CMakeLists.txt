cmake_minimum_required(VERSION 3.16)

project("ImmortalShared" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_FILES
    Async.cpp
    Async.h
    DLLLoader.cpp
    DLLLoader.h
    Log.cpp
    Log.h)

set(PROJECT_FILES
    ${SRC_FILES})

source_group("\\" FILE ${SRC_FILES})

add_library(${PROJECT_NAME} STATIC
    ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(${PROJECT_NAME} PUBLIC
    slapi
    spdlog_header_only)
