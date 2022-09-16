cmake_minimum_required(VERSION 3.16)

project(D3D12Sample LANGUAGES CXX)

set(SRC_FILES
    src/D3D12Sample.cpp
    src/D3D12Sample.h
    )

add_executable(${PROJECT_NAME}
    ${SRC_FILES})

source_group("\\" FILES ${SRC_FILES})

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    Immortal
)

target_link_runtime(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR})
