#pragma once

#include "Common.h"
#include "Format.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class VideoSession
{
public:
    using Primitive = VkVideoSessionKHR;
    VKCPP_OPERATOR_HANDLE()

public:
    VideoSession(Device* device, Format format, VkExtent2D extend, const VkVideoProfileInfoKHR* pVideoProfile, uint32_t maxReferencePicturesSlotsCount = 32, uint32_t maxReferencePicturesActiveCount = 32);

public:
    const VkVideoSessionParametersKHR &GetParameters() const
    {
        return parameters;
    }

private:
    Device *device{ nullptr };

    VkVideoSessionParametersKHR parameters{ VK_NULL_HANDLE };
};

}   
}
