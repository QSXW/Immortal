#include "Sampler.h"

namespace Immortal
{
namespace Vulkan
{

Sampler::Sampler(Device *device, const VkSamplerCreateInfo &info) :
    device{ device }
{
    Check(vkCreateSampler(device->Handle(), &info, nullptr, &handle));
}

Sampler::~Sampler()
{
    if (!device)
    {
        return;
    }
    IfNotNullThen(vkDestroySampler, device->Handle(), handle);
}

Sampler::Sampler(Sampler &&other) :
    device{ other.device },
    handle{ other.handle }
{
    other.device = nullptr;
    other.handle = VK_NULL_HANDLE;
}

Sampler &Sampler::operator =(Sampler &&other)
{
    if (this == &other)
    {
        return *this;
    }

    device = other.device;
    handle = other.handle;

    other.device = nullptr;
    other.handle = VK_NULL_HANDLE;

    return *this;
}

}
}
