cmake_minimum_required(VERSION 3.16)

set (CMAKE_VERBOSE_MAKEFILE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("ImmortalVision")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(VISION_CORE_FILES
    Codec.h
    CodedFrame.cpp
    CodedFrame.h
    Demuxer.h
    Image.cpp
    Image.h
    Picture.cpp
    Picture.h
    Types.h
    VisionPrecompiledHeader.h)
list(TRANSFORM VISION_CORE_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

set(VISION_AUDIO_FILES
    WAV.cpp
    WAV.h)
list(TRANSFORM VISION_AUDIO_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Audio/")

set(COMMON_FILES
    Animator.h
    BitTracker.h
    Checksum.cpp
    Checksum.h
    Error.h
    NetworkAbstractionLayer.h
    SamplingFactor.h)
list(TRANSFORM COMMON_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Common/")

set(DEMUX_FILES
    FFDemuxer.cpp
    FFDemuxer.h
    IVFDemuxer.cpp
    IVFDemuxer.h)
list(TRANSFORM DEMUX_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Demux/")

set(VISION_INTERFACE_FILES
    MFXCodec.cpp
    MFXCodec.h)
list(TRANSFORM VISION_INTERFACE_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Interface/")

set(LOOKUP_TABLE_FILE
    LookupTable.cpp
    LookupTable.h)
list(TRANSFORM LOOKUP_TABLE_FILE PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/LookupTable/")

set(IMAGE_CODEC_FILES
    BMP.cpp
    BMP.h
    Exif.cpp
    Exif.h
    Helper.h
    ImageCodec.h
    JPEG.cpp
    JPEG.h
    STBCodec.cpp
    STBCodec.h
    MFXJpegCodec.cpp
    MFXJpegCodec.h
    NVJpegCodec.cpp
    NVJpegCodec.h
    PPM.cpp
    PPM.h
    Raw.cpp
    Raw.h
    RawSpeed.h
    OpenCVCodec.cpp
    OpenCVCodec.h
    TurboJpegCodec.cpp
    TurboJpegCodec.h)
list(TRANSFORM IMAGE_CODEC_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Image/")

set(VIDEO_CODEC_FILES
    DAV1DCodec.cpp
    DAV1DCodec.h
    FFCodec.cpp
    FFCodec.h
    HEVC.cpp
    HEVC.h
    Video.h)
list(TRANSFORM VIDEO_CODEC_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Video/")

set(D3D12_VIDEO_FILES
    HEVCCodec.cpp
    HEVCCodec.h)
list(TRANSFORM D3D12_VIDEO_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Video/D3D12/")

set(VULKAN_VIDEO_FILES
    HEVC.cpp
    HEVC.h)
list(TRANSFORM VULKAN_VIDEO_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Video/Vulkan/")

set(PROCESSING_FILES
    ColorSpace.cpp
    ColorSpace.h)
list(TRANSFORM PROCESSING_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Processing/")

set(EXTERNAL_FILES
    External.cpp
    stb_image.h)
list(TRANSFORM EXTERNAL_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/External/")

source_group("\\"                FILES ${VISION_CORE_FILES}      )
source_group("\\Audio\\"         FILES ${VISION_AUDIO_FILES}    )
source_group("\\Common\\"        FILES ${COMMON_FILES}          )
source_group("\\Demux\\"         FILES ${DEMUX_FILES}           )
source_group("\\Interface\\"     FILES ${VISION_INTERFACE_FILES})
source_group("\\LookupTable\\"   FILES ${LOOKUP_TABLE_FILE}     )
source_group("\\Image\\"         FILES ${IMAGE_CODEC_FILES}     )
source_group("\\Video\\"         FILES ${VIDEO_CODEC_FILES}     )
source_group("\\Video\\D3D12\\"  FILES ${D3D12_VIDEO_FILES}     )
source_group("\\Video\\Vulkan\\" FILES ${VULKAN_VIDEO_FILES}    )
source_group("\\External\\"      FILES ${EXTERNAL_FILES}        )
source_group("\\Processing\\"    FILES ${PROCESSING_FILES}      )

set(VISION_FILES
    ${VISION_CORE_FILES}
    ${VISION_AUDIO_FILES}
    ${COMMON_FILES}
    ${DEMUX_FILES}
    ${EXTERNAL_FILES}
    ${VISION_INTERFACE_FILES}
    ${LOOKUP_TABLE_FILE}
    ${IMAGE_CODEC_FILES}
    ${VIDEO_CODEC_FILES}
    ${VULKAN_VIDEO_FILES}
    ${PROCESSING_FILES}
    $<$<C_COMPILER_ID:MSVC>:${D3D12_VIDEO_FILES}>)

add_library(${PROJECT_NAME} STATIC ${VISION_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    PUBLIC
        slapi
        ExternalVision
        ImmortalShared
        ImmortalGraphics)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..)

if (NOT APPLE)
    target_precompile_headers(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/VisionPrecompiledHeader.h>)
endif()
