#pragma once

#include "Graphics/Sampler.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class IMMORTAL_API Sampler : public SuperSampler, public NonDispatchableHandle
{
public:
	using Primitive = D3D12_CPU_DESCRIPTOR_HANDLE;

	Sampler(Device *device, Filter filter, AddressMode addressMode, CompareOperation compareOperation = {}, float minLod = 0.0f, float maxLod = 1.0f);

	virtual ~Sampler() override;

public:
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const
	{
		return handle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const
	{
		return handle;
	}

protected:
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
};
}
}
