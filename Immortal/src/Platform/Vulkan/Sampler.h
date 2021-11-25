#pragma once

#include "Common.h"
#include "Render/Texture.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Sampler
{
public:
    Sampler() = default;

    Sampler(Device *device, const VkSamplerCreateInfo &info);

    Sampler(Device *device, const Immortal::Texture::Description &desc);

    Sampler(Sampler &&other);

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

    operator VkSampler() const
    {
        return handle;
    }

private:
    Device *device{ nullptr };

    VkSampler handle{ VK_NULL_HANDLE };
};

}
}
