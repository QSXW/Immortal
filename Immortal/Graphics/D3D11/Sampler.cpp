#include "Sampler.h"
#include "Device.h"

namespace Immortal
{
namespace D3D11
{

static inline D3D11_TEXTURE_ADDRESS_MODE CAST(AddressMode addressMode)
{
	switch (addressMode)
	{
		case AddressMode::BorderColor:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		case AddressMode::Mirror:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case AddressMode::Clamp:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case AddressMode::Wrap:
		default:
			return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

Sampler::Sampler()
{

}

Sampler::Sampler(Device *device, Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod)
{
	D3D11_SAMPLER_DESC desc = {
		.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER,
		.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER,
		.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER,
		.MipLODBias     = 0,
		.MaxAnisotropy  = 1,
	    .ComparisonFunc = D3D11_COMPARISON_FUNC(compareOperation),
		.BorderColor    = { 1.0f, 1.0f, 1.0f, 1.0f },
		.MinLOD         = minLod,
		.MaxLOD         = maxLod,
	};
	
	if (filter == Filter::Nearest)
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	desc.AddressU = CAST(addressMode);
	desc.AddressV = desc.AddressU;
	desc.AddressW = desc.AddressU;
	
	Check(device->CreateSamplerState(&desc, &handle));
}

Sampler::~Sampler()
{
	handle.Reset();
}

}
}
