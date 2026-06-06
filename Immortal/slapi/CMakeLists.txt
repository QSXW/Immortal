cmake_minimum_required(VERSION 3.16)

project("slapi" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_FILES
    slapi.h
    slcpuid.h
    slintrinsic.h)

set(PROJECT_FILES
    ${SRC_FILES})

source_group("\\" FILE ${SRC_FILES})

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<C_COMPILER_ID:GNU>:-march=native>
)

add_custom_target("sl"
    SOURCES
        ${SRC_FILES}
    )
