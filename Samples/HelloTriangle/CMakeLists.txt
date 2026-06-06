cmake_minimum_required(VERSION 3.16)

project("HelloTriangle" LANGUAGES CXX)

set(SRC_FILES
    HelloTriangle.cpp)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)

source_group("\\" FILES ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    ImmortalGraphics
)

target_link_runtime(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Examples")
