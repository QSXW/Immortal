#pragma once

#include "Common.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Buffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Sampler.h"

namespace Immortal
{
namespace Metal
{

class Device;
class Pipeline;
class IMMORTAL_API DescriptorSet : public SuperDescriptorSet
{
public:
	DescriptorSet(Device *device, Pipeline *pipeline);

    virtual ~DescriptorSet() override;

	virtual void Set(uint32_t slot, SuperBuffer *buffer) override;

	virtual void Set(uint32_t slot, SuperTexture *texture) override;

	virtual void Set(uint32_t slot, SuperSampler *sampler) override;

public:
	void Set(MTL::RenderCommandEncoder *renderCommandEncoder);

	void Set(MTL::ComputeCommandEncoder *computeEncodeEncoder);

protected:
	std::vector<MTL::Buffer *> buffers;

	std::vector<MTL::Texture *> textures;

	std::vector<MTL::SamplerState *> samplers;
};

}
}
