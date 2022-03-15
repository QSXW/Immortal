#include "HEVC.h"

namespace Immortal
{
namespace Vision
{
namespace Vulkan
{

static inline VkVideoComponentBitDepthFlagBitsKHR SelectBitDepth(int bitDepth)
{
    switch (bitDepth)
    {
    case 8:
        return VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;

    case 10:
        return VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;

    case 12:
        return VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR;

    default:
        return VK_VIDEO_COMPONENT_BIT_DEPTH_INVALID_KHR;
    }
}

static inline VkVideoChromaSubsamplingFlagBitsKHR SelectChromaSampling(Format format)
{
    switch (Format::ValueType{ format })
    {
    case Format::YUV420P:
        return VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;

    case Format::YUV422P:
        return VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR;

    case Format::YUV444P:
        return VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR;

    default:
        return VK_VIDEO_CHROMA_SUBSAMPLING_INVALID_BIT_KHR;
        break;
    }
}

CodecError HEVCCodec::Decode(const std::vector<uint8_t> &rbsp)
{
    Super::Decode(rbsp);

    desc.width  = sps->pic_width_in_luma_samples;
    desc.height = sps->pic_height_in_luma_samples;

    VkVideoProfileKHR profile{};
    profile.pNext               = nullptr;
    profile.sType               = (VkStructureType)1000023000; /* VK_STRUCTURE_TYPE_VIDEO_PROFILE_KHR */
    profile.lumaBitDepth        = SelectBitDepth(sps->bit_depth_luma);
    profile.chromaBitDepth      = SelectBitDepth(sps->bit_depth_chroma);
    profile.chromaSubsampling   = SelectChromaSampling(desc.format);
    profile.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_EXT;

    if (!session)
    {
        session.reset(new VideoSession{
            Device::That,
            desc.format,
            VkExtent2D{ U32(desc.width), U32(desc.height) },
            &profile,
            vps->vps_max_dec_pic_buffering[0]
            });
    }

    StdVideoDecodeH265PictureInfo pictureInfo{};
    pictureInfo.vps_video_parameter_set_id  = vps->vps_video_parameter_set_id;
    pictureInfo.sps_seq_parameter_set_id    = sps->sps_seq_parameter_set_id;
    pictureInfo.pps_pic_parameter_set_id    = pps->pps_pic_parameter_set_id;
    pictureInfo.num_short_term_ref_pic_sets = sps->num_short_term_ref_pic_sets;

    VkVideoDecodeH265PictureInfoEXT pictureInfoExt{};
    pictureInfoExt.pNext           = nullptr;
    pictureInfoExt.sType           = (VkStructureType)1000187005; /* VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_EXT */
    pictureInfoExt.pStdPictureInfo = &pictureInfo;

    return CodecError::Succeed;
}

}
}
}
