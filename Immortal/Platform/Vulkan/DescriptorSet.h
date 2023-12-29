#pragma once

#include "Common.h"
#include "Descriptor.h"
#include "Handle.h"
#include "Render/DescriptorSet.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Buffer;
class Pipeline;
class Texture;
class Sampler;
class DescriptorSet : public SuperDescriptorSet, public Handle<VkDescriptorSet>
{
public:
    using Super = SuperDescriptorSet;
	VKCPP_SWAPPABLE(DescriptorSet)

public:
	DescriptorSet(Device *device = nullptr, Pipeline *pipeline = nullptr);

    virtual ~DescriptorSet() override;

    virtual void Set(uint32_t slot, SuperBuffer *buffer) override;

	virtual void Set(uint32_t slot, SuperTexture *texture) override;

	virtual void Set(uint32_t slot, SuperSampler *sampler) override;

    void Swap(DescriptorSet &other)
    {
		Handle::Swap(other);
        std::swap(device, other.device);
    }

protected:
    Device *device{ nullptr };

    LightArray<VkWriteDescriptorSet> writeDescriptorSets;

    VkDescriptorUpdateTemplate descriptorUpdateTemplate;
};

}
}
