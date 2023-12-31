#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/Sampler.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Sampler : public SuperSampler, public Handle<VkSampler>
{
public:

public:
    Sampler();

    Sampler(Device *device, const VkSamplerCreateInfo *pCreateInfo);

    Sampler(Device *device, Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod);

    virtual ~Sampler() override;

    void Release();

public:
    void Swap(Sampler &other)
    {
        Handle::Swap(other);
        std::swap(device, other.device);
    }

private:
    Device *device;
};

}
}
