cmake_minimum_required(VERSION 3.10)

project("slCodec" LANGUAGES CXX)

# Media
set(CORE_FILES
    Types.h
    RF.h
    BMP.cpp
    BMP.h
    ColorSpace.cpp
    ColorSpace.h
    Checksum.cpp
    Checksum.h
    Decoder.h
    OpenCVCodec.h
    Stream.h
    STBCodec.cpp
    STBCodec.h
    StillPicture.h
)

set(STB_IMAGE_FILES
    stb_image/stb_image.cpp
    stb_image/stb_image.h)

source_group("slCodec\\" FILES ${CORE_FILES})
source_group("slCodec\\stb_image\\" FILES ${STB_IMAGE_FILES})

add_library(${PROJECT_NAME} STATIC
    ${CORE_FILES}
    ${STB_IMAGE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/Immortal
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    slapi
    glm
    glad
    vulkan
    spdlog::spdlog
)
