#include "VideoSession.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

static inline Queue::Type SelectQueueType(VkVideoCodecOperationFlagBitsKHR codecOperation)
{
    switch (codecOperation)
    {
    case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_EXT:
    case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_EXT:
        return Queue::Type::VideoDecoding;

    case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT:
        return Queue::Type::VideoEncoding;

    default:
        return Queue::Type::None;
    }
}

static void *SelectEXT(VkVideoCodecOperationFlagBitsKHR codecOperation)
{
    static const VkExtensionProperties h264StdExtensionVersion{ 
        VK_STD_VULKAN_VIDEO_CODEC_H264_EXTENSION_NAME,
        VK_STD_VULKAN_VIDEO_CODEC_H264_SPEC_VERSION
    };

    static const VkExtensionProperties h265StdExtensionVersion{ 
        VK_STD_VULKAN_VIDEO_CODEC_H265_EXTENSION_NAME,
        VK_STD_VULKAN_VIDEO_CODEC_H265_SPEC_VERSION
    };

    static struct {
        VkVideoDecodeH264SessionCreateInfoEXT h264Decoder{
            VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_CREATE_INFO_EXT,
            nullptr,
            0,
            &h264StdExtensionVersion
        };

        VkVideoEncodeH264SessionCreateInfoEXT h264Encoder{
            VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_EXT,
            nullptr,
            0,
            VkExtent2D{ 0, 0 },
            &h264StdExtensionVersion
        };

        VkVideoDecodeH265SessionCreateInfoEXT h265Decoder{
            VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_CREATE_INFO_EXT,
            nullptr,
            0,
            &h265StdExtensionVersion
        };
    } createInfo;

    switch (codecOperation)
    {
    case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_EXT:
        return &createInfo.h265Decoder;

    case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_EXT:
        return &createInfo.h264Decoder;

    case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT:
        return &createInfo.h264Encoder;

    default:
        return nullptr;
    }
}

VideoSession::VideoSession(Device *device, Format format, VkExtent2D extend, const VkVideoProfileKHR *pVideoProfile, uint32_t maxReferencePicturesSlotsCount, uint32_t maxReferencePicturesActiveCount) :
    device{ device }
{
    VkVideoSessionCreateInfoKHR createInfo{};
    createInfo.sType                           = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR;
    createInfo.pNext                           = SelectEXT(pVideoProfile->videoCodecOperation);
    createInfo.pVideoProfile                   = pVideoProfile;
    createInfo.queueFamilyIndex                = device->QueueFailyIndex(SelectQueueType(pVideoProfile->videoCodecOperation));
    createInfo.pictureFormat                   = Format{ Format::RGBA8 };
    createInfo.referencePicturesFormat         = createInfo.pictureFormat;
    createInfo.maxCodedExtent                  = extend;
    createInfo.maxReferencePicturesSlotsCount  = maxReferencePicturesSlotsCount;
    createInfo.maxReferencePicturesActiveCount = maxReferencePicturesSlotsCount;

    Check(device->Create(&createInfo, &handle));
}

}
}
