#include "Sampler.h"
#include "Device.h"

namespace Immortal
{
namespace D3D11
{

Sampler::Sampler(Device *device, const Texture::Description &_desc)
{
	D3D11_SAMPLER_DESC desc = {
	    .Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
	    .AddressU       = D3D11_TEXTURE_ADDRESS_BORDER,
	    .AddressV       = D3D11_TEXTURE_ADDRESS_BORDER,
	    .AddressW       = D3D11_TEXTURE_ADDRESS_BORDER,
		.MipLODBias     = 0,
		.MaxAnisotropy  = 1,
	    .ComparisonFunc = D3D11_COMPARISON_NEVER,
		.BorderColor    = { 1.0f, 1.0f, 1.0f, 1.0f },
		.MinLOD         = 0,
		.MaxLOD         = D3D11_FLOAT32_MAX,
	};

	if (_desc.filter == Filter::Nearest)
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
	if (_desc.Anisotropic)
	{
		desc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	}
	if (_desc.wrap == Wrap::Clamp || _desc.wrap == Wrap::BorderColor)
	{
		desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	}
	if (_desc.wrap == Wrap::Mirror || _desc.wrap == Wrap::Repeat)
	{
		desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	}

	desc.AddressV = desc.AddressU;
	desc.AddressW = desc.AddressU;
	
	Check(device->CreateSamplerState(&desc, &handle));
}

} 
}
