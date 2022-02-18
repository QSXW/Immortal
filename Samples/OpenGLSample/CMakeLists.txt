cmake_minimum_required(VERSION 3.21)

project(OpenGLSample LANGUAGES CXX)

set(SRC_FILES
    src/OpenGLSample.cpp
    src/OpenGLSample.h
    src/RenderLayer.h
    )

source_group("\\" FILES ${SRC_FILES})

set(PROJECT_FILES
    ${SRC_FILES})

source_group("\\" FILES ${PROJECT_FILES})

add_executable(${PROJECT_NAME}
    ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    Immortal)

target_sources(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

file(COPY ${WORKSPACE}/Assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${ASSIMP_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${OPENCV_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
MESSAGE("The libraries has been copied into" ${CMAKE_CURRENT_BINARY_DIR} --> ${ASSIMP_SHARED} ${OPENCV_SHARED} ${MONO_SHARED})
