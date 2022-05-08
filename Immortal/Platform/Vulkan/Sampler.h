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
    using Primitive = VkSampler;
    VKCPP_OPERATOR_HANDLE()

public:
    Sampler() = default;

    Sampler(Device *device, const VkSamplerCreateInfo &info);

    Sampler(Device *device, const Texture::Description &desc, uint32_t mipLevels = 1);

    Sampler(Sampler &&other);

    Sampler &operator =(Sampler &&other);

    ~Sampler();

private:
    Device *device{ nullptr };
};

}
}
