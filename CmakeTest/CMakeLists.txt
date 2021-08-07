cmake_minimum_required(VERSION 3.10)

project(CmakeTest)

add_executable(${PROJECT_NAME} test.cpp volk.h)

file(COPY ${ASSIMP_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${OPENCV_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
MESSAGE(动态库已写入文件夹 ${CMAKE_CURRENT_BINARY_DIR} --> ${ASSIMP_SHARED} ${OPENCV_SHARED})

target_link_libraries(${PROJECT_NAME}
    glm
    glad
    glfw
    assimp
    vulkan
    spdlog
    imgui
    opencv)