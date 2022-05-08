#include "VideoSession.h"
#include "Device.h"

#include <vk_video/vulkan_video_codecs_common.h>
#include <vk_video/vulkan_video_codec_h264std.h>
#include <vk_video/vulkan_video_codec_h265std.h>
#include <vk_video/vulkan_video_codec_h264std_encode.h>
#include <vk_video/vulkan_video_codec_h264std_decode.h>
#include <vk_video/vulkan_video_codec_h265std_encode.h>
#include <vk_video/vulkan_video_codec_h265std_decode.h>

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

VideoSession::VideoSession(Device *device, Format format, VkExtent2D extend, const VkVideoProfileKHR *pVideoProfile, uint32_t maxReferencePicturesSlotsCount, uint32_t maxReferencePicturesActiveCount) :
    device{ device }
{
    VkVideoSessionCreateInfoKHR createInfo{};
    createInfo.sType                           = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR;
    createInfo.pNext                           = nullptr;
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
