cmake_minimum_required(VERSION 3.16)

project("ImmortalEditor" LANGUAGES CXX)

set(PANEL_FILES
    Panel/HierarchyGraphics.h
    Panel/Navigator.h
    Panel/PropertyManager.h
    Panel/Tools.h)

set(SRC_FILES
    src/Main.cpp
    src/RenderLayer.h)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${PANEL_FILES}
)

source_group("\\Panel\\" FILES ${PANEL_FILES})
source_group("\\" FILES ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    Immortal
    ImmortalHLSL)

# INTERFACE libs do not pull custom targets; depend on the aggregate so copy + DXIL always run before link.
add_dependencies(${PROJECT_NAME} ImmortalHLSL_build_shaders)

immortal_target_link_runtime_dependency(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Examples")
