#pragma once

#include "Common.h"
#include "Render/Sampler.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class IMMORTAL_API Sampler : public SuperSampler
{
public:
    using Primitive = ID3D11SamplerState;
    D3D11_OPERATOR_HANDLE()

public:
	Sampler();

	Sampler(Device *device, Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod);

    virtual ~Sampler() override;

public:
    void Swap(Sampler &other)
    {
		std::swap(handle, other.handle);
    }
};

}
}
