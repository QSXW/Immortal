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
    VideoSession(Device *device, Format format, VkExtent2D extend, const VkVideoProfileKHR *pVideoProfile, uint32_t maxReferencePicturesSlotsCount = 32, uint32_t maxReferencePicturesActiveCount = 32);

public:
    operator VkVideoSessionKHR() const
    {
        return handle;
    }

    const VkVideoSessionParametersKHR &GetParameters() const
    {
        return parameters;
    }

private:
    Device *device{ nullptr };

    VkVideoSessionKHR handle{ VK_NULL_HANDLE };

    VkVideoSessionParametersKHR parameters{ VK_NULL_HANDLE };
};

}   
}
