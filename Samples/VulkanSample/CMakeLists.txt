cmake_minimum_required(VERSION 3.21)

project(VulkanSample LANGUAGES CXX)

set(PANEL_FILES
    Panel/Navigator.h)

set(SRC_FILES
    src/VulkanSample.cpp
    src/VulkanSample.h
    src/NetLayer.h
    src/RenderLayer.h
    )

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${PANEL_FILES}
)

source_group("\\Panel\\" FILES ${PANEL_FILES})
source_group("\\" FILES ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    Immortal)

file(COPY ${WORKSPACE}//Assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${ASSIMP_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${OPENCV_SHARED} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY ${MONO_SHARED}   DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
MESSAGE(
    "The libraries has been copied into"
    ${CMAKE_CURRENT_BINARY_DIR}
    " --> "
    ${ASSIMP_SHARED}
    ${OPENCV_SHARED}
    #${MONO_SHARED}
    )
