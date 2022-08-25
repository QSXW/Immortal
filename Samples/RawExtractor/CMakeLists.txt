cmake_minimum_required(VERSION 3.16)

project("RawExtractor" LANGUAGES CXX)

set(SRC_FILES
    RawExtractor.cpp
    RawExtractor.h
    )

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)

source_group("\\" FILES ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    Immortal
)

file(COPY ${WORKSPACE}//Assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${ASSIMP_SHARED}     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${OPENCV_SHARED}     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${MONO_SHARED}       DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${DAV1D_SHARED}      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${FFMPEG_SHARED}     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

MESSAGE(
    "The libraries has been copied into"
    ${CMAKE_CURRENT_BINARY_DIR}
    " --> "
    ${ASSIMP_SHARED}
    ${OPENCV_SHARED}
    ${MONO_SHARED}
    ${DAV1D_SHARED}
    )
