cmake_minimum_required(VERSION 3.21)

set ( CMAKE_VERBOSE_MAKEFILE on  )

project("slMedia" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC_FILES 
    Types.h
    RF.h
    BMP.cpp
    BMP.h
    Checksum.cpp
    Checksum.h
    Decoder.h
    Stream.h
    StillPicture.h
    )

source_group("\\" FILES ${SRC_FILES})

add_library(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

target_link_libraries(${PROJECT_NAME}
    slapi
    )
