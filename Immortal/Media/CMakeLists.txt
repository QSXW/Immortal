cmake_minimum_required(VERSION 3.10)

project("Vision" LANGUAGES CXX)

set(CORE_FILES
    Types.h
    RF.h
    Stream.h)

set(COMMON_FILES
    Common/Checksum.cpp
    Common/Checksum.h
    Common/Error.h
    Common/BitTracker.h)

set(INTERFACE_FILES
    Interface/Codec.h)

set(IMAGE_CODEC_FILES
    Image/BMP.cpp
    Image/BMP.h
    Image/Image.h
    Image/STBCodec.cpp
    Image/STBCodec.h
    Image/OpenCVCodec.h)

set(PROCESSING_FILES
    Processing/ColorSpace.cpp
    Processing/ColorSpace.h)

set(EXTERNAL_FILES
    External/stb_image/stb_image.cpp
    External/stb_image/stb_image.h)

source_group("\\" FILES ${CORE_FILES})
source_group("\\Common\\" FILES ${COMMON_FILES})
source_group("\\Interface\\" FILES ${INTERFACE_FILES})
source_group("\\Image\\" FILES ${IMAGE_CODEC_FILES})
source_group("\\External\\" FILES ${EXTERNAL_FILES})

add_library(${PROJECT_NAME} STATIC
    ${CORE_FILES}
    ${COMMON_FILES}
    ${EXTERNAL_FILES}
    ${INTERFACE_FILES}
    ${IMAGE_CODEC_FILES}
    ${PROCESSING_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/Immortal
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    slapi
    external
    Immortal
)
