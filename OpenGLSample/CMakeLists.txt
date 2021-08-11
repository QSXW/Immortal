cmake_minimum_required(VERSION 3.20)

project(OpenGLSample LANGUAGES CXX)

set(SRC_FILES 
    src/OpenGLSample.cpp
    src/OpenGLSample2D.h
    src/OpenGLSample2D.cpp
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

file(COPY ${ASSIMP_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${OPENCV_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${MONO_SHARED}   DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
MESSAGE(动态库已写入文件夹 ${CMAKE_CURRENT_BINARY_DIR} --> ${ASSIMP_SHARED} ${OPENCV_SHARED} ${MONO_SHARED})
