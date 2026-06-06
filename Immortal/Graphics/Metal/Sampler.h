#pragma once

#include "Handle.h"
#include "Graphics/Sampler.h"
#include "Metal/Common.h"
#include "Metal/MTLSampler.hpp"

namespace Immortal
{
namespace Metal
{

class Device;
class IMMORTAL_API Sampler : public SuperSampler, public Handle<MTL::SamplerState>
{
public:
	METAL_SWAPPABLE(Sampler)

public:
	Sampler();

	Sampler(Device *device, Filter filter, AddressMode addressMode, CompareOperation compareOperation = {}, float minLod = 0.0f, float maxLod = 1.0f);

	virtual ~Sampler() override;

public:
	void Swap(Sampler &other)
	{
		Handle::Swap(other);
	}
};
}
}
