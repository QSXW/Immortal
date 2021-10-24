#pragma once

#include "Common.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

class Sampler
{
public:
    Sampler() = default;

    Sampler(Device *device, const VkSamplerCreateInfo &info);

    Sampler::Sampler(Sampler &&other);

    Sampler &operator =(Sampler &&other);

    ~Sampler();

    VkSampler &Handle()
    {
        return handle;
    }

    operator VkSampler&()
    {
        return handle;
    }

private:
    Device *device{ nullptr };

    VkSampler handle{ VK_NULL_HANDLE };
};

}
}
