cmake_minimum_required(VERSION 3.20)

project(ImmortalEditor LANGUAGES CXX)

message(STATUS "Start configure Immortal Editor...")

set(SRC_FILES 
    src/Editor.cpp
    src/EditorLayer.cpp
    src/EditorLayer.h
    
    )

set(PANEL_FILES
    src/Panel/SceneHierarchyPanel.cpp
    src/Panel/SceneHierarchyPanel.h
    )

set(PROJECT_FILES
    ${SRC_FILES}
    ${PANEL_FILES}
    )

add_executable(${PROJECT_NAME} ${PROJECT_FILES})

source_group("\\" FILES ${SRC_FILES})
source_group("\\Panel\\" FILES ${PANEL_FILES})
source_group("\\" FILES ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR})
    
target_link_libraries(${PROJECT_NAME}
    Immortal)

file(COPY imgui.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../Assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${ASSIMP_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${OPENCV_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${MONO_SHARED}   DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
MESSAGE(STATUS "The runtime library has been write to" ${CMAKE_CURRENT_BINARY_DIR} --> ${ASSIMP_SHARED} ${OPENCV_SHARED} ${MONO_SHARED})

message(STATUS "Configure Immortal Editor with success")
