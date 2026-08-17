cmake_minimum_required(VERSION 3.16)

project("HelloWorkGraph" LANGUAGES CXX)

set(SRC_FILES
HelloWorkGraph.cpp)

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

immortal_target_link_runtime_dependency(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Examples")
