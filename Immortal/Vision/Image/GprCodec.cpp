#include "GprCodec.h"

#include "Raw.h"

#include <gpr.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <utility>

namespace Immortal
{
namespace Vision
{

namespace
{

std::mutex &GetGprSdkMutex()
{
    static std::mutex mutex;
    return mutex;
}

GPR_RGB_RESOLUTION GetGprThumbnailResolution(
    const gpr_parameters &parameters,
    uint32_t requestedWidth)
{
    if (!requestedWidth)
    {
        return GPR_RGB_RESOLUTION_SIXTEENTH;
    }

    const uint64_t longestEdge = std::max<uint64_t>(
        parameters.input_width,
        parameters.input_height);
    if ((longestEdge + 15) / 16 >= requestedWidth)
    {
        return GPR_RGB_RESOLUTION_SIXTEENTH;
    }
    if ((longestEdge + 7) / 8 >= requestedWidth)
    {
        return GPR_RGB_RESOLUTION_EIGHTH;
    }
    if ((longestEdge + 3) / 4 >= requestedWidth)
    {
        return GPR_RGB_RESOLUTION_QUARTER;
    }
    return GPR_RGB_RESOLUTION_HALF;
}

DisplayOrientation GetGprDisplayOrientation(GPR_ORIENTATION orientation)
{
    struct Transform
    {
        int hflip;
        int anticlockwiseRotation;
    };

    // GPR stores Adobe DNG orientation values, not TIFF/EXIF values.
    static constexpr Transform transforms[] = {
        { 0, 0 },
        { 0, -270 },
        { 0, -180 },
        { 0, -90 },
        { 1, 0 },
        { 1, -270 },
        { 1, -180 },
        { 1, -90 }
    };

    const int index = static_cast<int>(orientation);
    if (index < 0 || index >= static_cast<int>(sizeof(transforms) / sizeof(transforms[0])))
    {
        return {};
    }

    return {
        .hflip = transforms[index].hflip,
        .anticlockwiseRotation = transforms[index].anticlockwiseRotation
    };
}

DisplayOrientation ResolveGprDisplayOrientation(
    const String &sourcePath,
    const gpr_parameters &parameters)
{
    DisplayOrientation displayOrientation =
        GetGprDisplayOrientation(parameters.tuning_info.orientation);
    const bool usedPortraitFallback =
        parameters.tuning_info.orientation == ORIENTATION_NORMAL &&
        parameters.exif_info.scene_capture_type ==
            gpr_scene_capture_type_portrait &&
        parameters.input_width > parameters.input_height;
    if (usedPortraitFallback)
    {
        // HERO GPR files can leave DNG Orientation at Normal while marking
        // the sensor's landscape raster as a portrait capture in ExifIFD.
        displayOrientation.anticlockwiseRotation = -90;
    }

    LOG::DEBUG(
        "GPR display orientation: source=`{}`, SDK={}, SceneCaptureType={}, portrait fallback={}, result hflip={}, rotation={}",
        sourcePath,
        static_cast<int>(parameters.tuning_info.orientation),
        static_cast<int>(parameters.exif_info.scene_capture_type),
        usedPortraitFallback,
        displayOrientation.hflip,
        displayOrientation.anticlockwiseRotation);
    return displayOrientation;
}

DisplayOrientation ResolveGprThumbnailDisplayOrientation(
    const String &sourcePath,
    const gpr_parameters &parameters)
{
    DisplayOrientation displayOrientation =
        ResolveGprDisplayOrientation(sourcePath, parameters);

    // The SDK's WaveletToRGB path reverses X; the RAW/DNG path does not.
    displayOrientation.hflip = !displayOrientation.hflip;
    return displayOrientation;
}

class GprDecodeContext
{
public:
    GprDecodeContext() :
        allocator{ &std::malloc, &std::free },
        parameters{},
        dngBuffer{},
        rgbBuffer{}
    {
        gpr_parameters_set_defaults(&parameters);
    }

    ~GprDecodeContext()
    {
        if (dngBuffer.buffer)
        {
            allocator.Free(dngBuffer.buffer);
        }
        if (rgbBuffer.buffer)
        {
            allocator.Free(rgbBuffer.buffer);
        }
        gpr_parameters_destroy(&parameters, allocator.Free);
    }

    CodecError Parse(const CodedFrame &codedFrame)
    {
        if (!codedFrame.GetData() || !codedFrame.GetSize())
        {
            return CodecError::InvalidArguments;
        }

        gpr_buffer inputBuffer{
            const_cast<uint8_t *>(codedFrame.GetData()),
            codedFrame.GetSize()
        };
        if (!gpr_parse_metadata(&allocator, &inputBuffer, &parameters))
        {
            return CodecError::CorruptStream;
        }

        return CodecError::Success;
    }

    CodecError ConvertThumbnail(const CodedFrame &codedFrame, uint32_t requestedWidth)
    {
        CodecError error = Parse(codedFrame);
        if (error != CodecError::Success)
        {
            return error;
        }

        gpr_buffer inputBuffer{
            const_cast<uint8_t *>(codedFrame.GetData()),
            codedFrame.GetSize()
        };
        if (!gpr_convert_gpr_to_rgb(
                &allocator,
                GetGprThumbnailResolution(parameters, requestedWidth),
                8,
                &inputBuffer,
                &rgbBuffer) ||
            !rgbBuffer.buffer || !rgbBuffer.size ||
            !rgbBuffer.width || !rgbBuffer.height)
        {
            return CodecError::ExternalFailed;
        }

        return CodecError::Success;
    }

    CodecError Convert(const CodedFrame &codedFrame)
    {
        CodecError error = Parse(codedFrame);
        if (error != CodecError::Success)
        {
            return error;
        }

        gpr_buffer inputBuffer{
            const_cast<uint8_t *>(codedFrame.GetData()),
            codedFrame.GetSize()
        };
        const GPR_ORIENTATION sourceOrientation = parameters.tuning_info.orientation;
        parameters.tuning_info.orientation = ORIENTATION_NORMAL;
        const bool converted =
            gpr_convert_gpr_to_dng(&allocator, &parameters, &inputBuffer, &dngBuffer);
        parameters.tuning_info.orientation = sourceOrientation;
        if (!converted ||
            !dngBuffer.buffer || !dngBuffer.size)
        {
            return CodecError::ExternalFailed;
        }

        return CodecError::Success;
    }

    CodedFrame GetDngFrame() const
    {
        return CodedFrame{ dngBuffer.buffer, dngBuffer.size };
    }

    const gpr_parameters &GetParameters() const
    {
        return parameters;
    }

    const gpr_rgb_buffer &GetRgbBuffer() const
    {
        return rgbBuffer;
    }

private:
    gpr_allocator allocator;
    gpr_parameters parameters;
    gpr_buffer dngBuffer;
    gpr_rgb_buffer rgbBuffer;
};

void SetGprDisplayOrientation(
    Picture &picture,
    const DisplayOrientation &displayOrientation)
{
    if (!picture)
    {
        return;
    }

    DisplayOrientation *property = picture.GetProperty<DisplayOrientation>();
    if (!property)
    {
        property = picture.AllocateProperty<DisplayOrientation>();
    }
    property->hflip = displayOrientation.hflip;
    property->anticlockwiseRotation = displayOrientation.anticlockwiseRotation;
}

}

GprCodec::GprCodec(Format outputFormat, const String &sourcePath) :
    format{ outputFormat },
    sourcePath{ sourcePath }
{

}

CodecError GprCodec::Decode(const CodedFrame &codedFrame)
{
    std::lock_guard lock{ GetGprSdkMutex() };
    picture = {};

    try
    {
        GprDecodeContext context;
        CodecError error = context.Convert(codedFrame);
        if (error != CodecError::Success)
        {
            LOG::ERR("Failed to convert GPR image to DNG");
            return error;
        }

        CodedFrame dngFrame = context.GetDngFrame();
        RawCodec rawCodec{ format };
        error = rawCodec.Decode(dngFrame);
        if (error != CodecError::Success)
        {
            LOG::ERR("Failed to decode the DNG converted from GPR");
            return error;
        }

        picture = rawCodec.GetPicture();
        SetGprDisplayOrientation(
            picture,
            ResolveGprDisplayOrientation(
                sourcePath,
                context.GetParameters()));
        return picture ? CodecError::Success : CodecError::ExternalFailed;
    }
    catch (...)
    {
        LOG::ERR("Unexpected exception while decoding GPR image");
        return CodecError::ExternalFailed;
    }
}

CodecError GprCodec::DecodeThumbnail(
    const CodedFrame &codedFrame,
    uint32_t requestedWidth)
{
    std::lock_guard lock{ GetGprSdkMutex() };
    picture = {};

    try
    {
        GprDecodeContext context;
        CodecError error = context.ConvertThumbnail(codedFrame, requestedWidth);
        if (error != CodecError::Success)
        {
            LOG::ERR("Failed to decode GPR thumbnail");
            return error;
        }

        const gpr_rgb_buffer &rgb = context.GetRgbBuffer();
        if (rgb.width > std::numeric_limits<uint32_t>::max() ||
            rgb.height > std::numeric_limits<uint32_t>::max() ||
            rgb.width > std::numeric_limits<size_t>::max() / rgb.height ||
            rgb.width * rgb.height > rgb.size / 3)
        {
            return CodecError::CorruptStream;
        }

        Picture thumbnail{
            static_cast<uint32_t>(rgb.width),
            static_cast<uint32_t>(rgb.height),
            Format::RGBA8,
            true
        };
        if (!thumbnail.GetData())
        {
            return CodecError::OutOfMemory;
        }

        const uint8_t *source = static_cast<const uint8_t *>(rgb.buffer);
        for (uint32_t y = 0; y < thumbnail.GetHeight(); ++y)
        {
            const uint8_t *sourceRow = source + size_t{ y } * thumbnail.GetWidth() * 3;
            uint8_t *destination = thumbnail.GetData() + size_t{ y } * thumbnail.GetStride();
            for (uint32_t x = 0; x < thumbnail.GetWidth(); ++x)
            {
                destination[x * 4 + 0] = sourceRow[x * 3 + 0];
                destination[x * 4 + 1] = sourceRow[x * 3 + 1];
                destination[x * 4 + 2] = sourceRow[x * 3 + 2];
                destination[x * 4 + 3] = 255;
            }
        }
        thumbnail.SetFlags(PictureFlags::FullRange);
        SetGprDisplayOrientation(
            thumbnail,
            ResolveGprThumbnailDisplayOrientation(
                sourcePath,
                context.GetParameters()));
        picture = std::move(thumbnail);
        return CodecError::Success;
    }
    catch (...)
    {
        LOG::ERR("Unexpected exception while decoding GPR thumbnail");
        return CodecError::ExternalFailed;
    }
}

CodecError GprCodec::DecodeHeader(CodedFrame &codedFrame, CodecInfo &info)
{
    std::lock_guard lock{ GetGprSdkMutex() };

    try
    {
        GprDecodeContext context;
        CodecError error = context.Parse(codedFrame);
        if (error != CodecError::Success)
        {
            return error;
        }

        const gpr_parameters &parameters = context.GetParameters();
        if (!parameters.input_width || !parameters.input_height)
        {
            return CodecError::CorruptStream;
        }

        const DisplayOrientation displayOrientation =
            ResolveGprDisplayOrientation(sourcePath, parameters);

        info = CodecInfo{
            .mediaType = MediaType::Video,
            .codecId = CodecId::RAW,
            .width = parameters.input_width,
            .height = parameters.input_height,
            .format = Format::BayerLayerRGGB,
            .bitRate = 0,
            .gopSize = 0,
            .framerate = { 0, 1 },
            .timeBase = { 0, 1 },
            .displayOrientation = displayOrientation
        };
        return CodecError::Success;
    }
    catch (...)
    {
        LOG::ERR("Unexpected exception while reading GPR image metadata");
        return CodecError::ExternalFailed;
    }
}

}
}
