#include "Sampler.h"

namespace Immortal
{
namespace D3D12
{

static inline D3D12_FILTER CAST(Filter filter)
{
	switch (filter)
	{
		case Filter::Linear:
		case Filter::Bilinear:
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		case Filter::Anisotropic:
			return D3D12_FILTER_MINIMUM_ANISOTROPIC;
	    case Filter::None:
		case Filter::Nearest:
		default:
			return D3D12_FILTER_MIN_MAG_MIP_POINT;
	}
}

static inline D3D12_TEXTURE_ADDRESS_MODE CAST(AddressMode addressMode)
{
	return D3D12_TEXTURE_ADDRESS_MODE(addressMode);
}

static inline D3D12_COMPARISON_FUNC CAST(CompareOperation compareOperation)
{
	return D3D12_COMPARISON_FUNC(compareOperation);
}

Sampler::Sampler(Device *device, Filter filter, AddressMode _addressMode, CompareOperation compareOperation, float minLod, float maxLod) :
    NonDispatchableHandle{ device },
    handle{}
{
	D3D12_TEXTURE_ADDRESS_MODE addressMode = CAST(_addressMode);
	D3D12_SAMPLER_DESC desc{
		.Filter         = CAST(filter),
		.AddressU       = addressMode,
		.AddressV       = addressMode,
		.AddressW       = addressMode,
		.MipLODBias     = 0,
		.MaxAnisotropy  = 1,
		.ComparisonFunc = CAST(compareOperation),
		.BorderColor    = { 0, 0, 0, 0 },
		.MinLOD         = minLod,
		.MaxLOD         = maxLod,
	};

	handle = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, &descriptorHeap);
	device->CreateSampler(&desc, &handle);
}

Sampler::~Sampler()
{
	if (handle)
	{
		device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, descriptorHeap, handle);
		handle = {};
	}
}

}
}
