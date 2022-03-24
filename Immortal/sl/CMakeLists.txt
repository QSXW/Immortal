cmake_minimum_required(VERSION 3.16)

project(slapi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_FILES
    sl.h
    slapi.h
    slcast.h
    slintrinsic.h)

set(PROJECT_FILES
    ${SRC_FILES})

source_group("\\" FILE ${PROJECT_FILES})

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
