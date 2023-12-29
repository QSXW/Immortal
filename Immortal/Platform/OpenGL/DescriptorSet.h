#pragma once

#include "Render/DescriptorSet.h"
#include "Render/LightGraphics.h"
#include "Descriptor.h"
#include "Common.h"

#include <vector>

namespace Immortal
{
namespace OpenGL
{

class Pipeline;
class IMMORTAL_API DescriptorSet : public SuperDescriptorSet
{
public:
	DescriptorSet(Pipeline *pipeline);

	virtual ~DescriptorSet() override;

	virtual void Set(uint32_t slot, SuperBuffer *buffer) override;

	virtual void Set(uint32_t slot, SuperTexture *texture) override;

	virtual void Set(uint32_t slot, SuperSampler *sampler) override;

	void BindDescriptorTable();

protected:
	std::vector<Descriptor> descriptors;

	uint32_t sampler;
};

}
}
