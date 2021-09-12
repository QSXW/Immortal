#include "Sampler.h"

namespace Immortal
{
namespace Vulkan
{
Sampler::Sampler(Device *device, const VkSamplerCreateInfo &info) :
    device{ device }
{
    Check(vkCreateSampler(device->Get<VkDevice>(), &info, nullptr, &handle));
}

Sampler::~Sampler()
{
    IfNotNullThen(vkDestroySampler, device->Get<VkDevice>(), handle);
}
}
}
