cmake_minimum_required(VERSION 3.21)

project(CmakeTest LANGUAGES CXX)

add_executable(${PROJECT_NAME} test.cpp )

file(COPY ${ASSIMP_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${OPENCV_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${MONO_SHARED}   DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
MESSAGE(动态库已写入文件夹 ${CMAKE_CURRENT_BINARY_DIR} --> ${ASSIMP_SHARED} ${OPENCV_SHARED} ${MONO_SHARED})

target_link_libraries(${PROJECT_NAME}
    glm
    glad
    glfw
    mono
    assimp
    vulkan
    spdlog
    imgui
    opencv
    Immortal)