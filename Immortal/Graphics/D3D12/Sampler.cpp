#include "Sampler.h"

namespace Immortal
{
namespace D3D12
{

static inline D3D12_FILTER_TYPE CAST_FILTER_TYPE(Filter filter)
{
	switch (filter)
	{
		case Filter::Linear:
		case Filter::Bilinear:
			return D3D12_FILTER_TYPE_LINEAR;
		case Filter::None:
		case Filter::Nearest:
			return D3D12_FILTER_TYPE_POINT;
		case Filter::Anisotropic:
		default:
			return D3D12_FILTER_TYPE_LINEAR;
	}
}

static inline D3D12_FILTER CAST(Filter mipFilter, Filter minFilter, Filter magFilter, CompareOperation compareOperation)
{
	if (mipFilter == Filter::Anisotropic || minFilter == Filter::Anisotropic || magFilter == Filter::Anisotropic)
	{
		return compareOperation == CompareOperation::Never ? D3D12_FILTER_ANISOTROPIC : D3D12_FILTER_COMPARISON_ANISOTROPIC;
	}
	const D3D12_FILTER_REDUCTION_TYPE reduction = compareOperation == CompareOperation::Never ?
		D3D12_FILTER_REDUCTION_TYPE_STANDARD :
		D3D12_FILTER_REDUCTION_TYPE_COMPARISON;
	return D3D12_ENCODE_BASIC_FILTER(
	    CAST_FILTER_TYPE(minFilter),
	    CAST_FILTER_TYPE(magFilter),
	    CAST_FILTER_TYPE(mipFilter),
	    reduction);
}

static inline D3D12_TEXTURE_ADDRESS_MODE CAST(AddressMode addressMode)
{
	return D3D12_TEXTURE_ADDRESS_MODE(addressMode);
}

static inline D3D12_COMPARISON_FUNC CAST(CompareOperation compareOperation)
{
	switch (compareOperation)
	{
		case CompareOperation::Never:          return D3D12_COMPARISON_FUNC_NEVER;
		case CompareOperation::Less:           return D3D12_COMPARISON_FUNC_LESS;
		case CompareOperation::Equal:          return D3D12_COMPARISON_FUNC_EQUAL;
		case CompareOperation::LessOrEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case CompareOperation::Greater:        return D3D12_COMPARISON_FUNC_GREATER;
		case CompareOperation::NotEqual:       return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case CompareOperation::GreaterOrEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case CompareOperation::Always:
		default:
			return D3D12_COMPARISON_FUNC_ALWAYS;
	}
}

Sampler::Sampler(Device *device, Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod) :
	Sampler{ device, filter, filter, filter, addressMode, compareOperation, minLod, maxLod }
{
}

Sampler::Sampler(Device *device, Filter mipFilter, Filter minFilter, Filter magFilter, AddressMode _addressMode, CompareOperation compareOperation, float minLod, float maxLod) :
    NonDispatchableHandle{ device },
    handle{}
{
	D3D12_TEXTURE_ADDRESS_MODE addressMode = CAST(_addressMode);
	D3D12_SAMPLER_DESC desc{
		.Filter         = CAST(mipFilter, minFilter, magFilter, compareOperation),
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
