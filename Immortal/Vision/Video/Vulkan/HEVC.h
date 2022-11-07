#pragma once

#ifndef HEVC_VULKAN_H__
#define HEVC_VULKAN_H__

#include "Vision/Video/HEVC.h"
#include "Platform/Vulkan/Common.h"
#include "Platform/Vulkan/Device.h"
#include "Platform/Vulkan/VideoSession.h"
#include <vk_video/vulkan_video_codec_h265std_decode.h>

namespace Immortal
{
namespace Vision
{
namespace Vulkan
{

using namespace Immortal::Vulkan;

class HEVCCodec : public SuperHEVCCodec
{
public:
    using Super = SuperHEVCCodec;

    struct ParameterSet
    {
        StdVideoH265SequenceParameterSet    sps;
        StdVideoH265SequenceParameterSetVui vui;
        StdVideoH265PictureParameterSet     pps;
        StdVideoH265ScalingLists            spsScalingLists;
        StdVideoH265ScalingLists            ppsScalingLists;
    };

public:
    virtual CodecError Decode(const std::vector<uint8_t> &rbsp) override;

private:
    void UpdateSequenceParameterSet();

    void UpdatePictureParameterSet();

    void TransferParameterSet();

private:
    URef<VideoSession> session;

    ParameterSet ps;
};

}
}
}

#endif
