#include "Texture.h"

#include "Render/Frame.h"
#include "Device.h"
#include "Buffer.h"

namespace Immortal
{
namespace D3D11
{

static inline uint32_t GetUsage(TextureType type)
{
	uint32_t usage = D3D11_BIND_SHADER_RESOURCE;
	if (type & TextureType::DepthStencilAttachment)
	{
		usage |= D3D11_BIND_DEPTH_STENCIL;
	}
	if (type & TextureType::ColorAttachment)
	{
		usage |= D3D11_BIND_RENDER_TARGET;
	}
	if (type & TextureType::Storage)
	{
		usage |= D3D11_BIND_UNORDERED_ACCESS;
	}

	return usage;
}

Texture::Texture(Device *device) :
    NonDispatchableHandle{ device }
{

}

Texture::Texture(Device *device, Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type) :
    NonDispatchableHandle{ device }
{
	SetMeta(width, height, mipLevels, arrayLayers);
	D3D11_TEXTURE2D_DESC desc = {
		.Width          = width,
		.Height         = height,
		.MipLevels      = mipLevels,
		.ArraySize      = arrayLayers,
		.Format         = format,
		.SampleDesc     = { .Count = 1, .Quality = 0, },
		.Usage          = D3D11_USAGE_DEFAULT,
		.BindFlags      = GetUsage(type),
		.CPUAccessFlags = 0,
		.MiscFlags      = mipLevels > 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0U,
    };

	if (mipLevels > 1)
	{
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}

    Check(device->CreateTexture2D(&desc, nullptr, &handle));

	ConstructResourceView();
}

Texture::Texture(Device *device, ComPtr<ID3D11Texture2D> pTexture) :
    NonDispatchableHandle{ device },
    handle{ pTexture }
{
	ConstructResourceView();
}

Texture::~Texture()
{
	handle.Reset();
}

void Texture::ConstructResourceView()
{
	D3D11_TEXTURE2D_DESC desc{};
	handle->GetDesc(&desc);

	if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {
			.Format        = desc.Format,
			.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
			.Texture2D     = { .MostDetailedMip = 0, .MipLevels = desc.MipLevels, }
		};

		Check(device->CreateShaderResourceView(handle.Get(), &viewDesc, descriptor.AddressOf()));
	}
	if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {
			.Format        = desc.Format,
			.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
			.Texture2D     = { .MipSlice = 0 }
		};

		Check(device->CreateRenderTargetView(handle.Get(), &viewDesc, rtv.AddressOf()));
	}
	if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc = {
		    .Format        = desc.Format,
		    .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		    .Texture2D     = { .MipSlice = 0 }
		};

		Check(device->CreateDepthStencilView(handle.Get(), &viewDesc, dsv.AddressOf()));
	}
	if (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
			.Format        = desc.Format,
			.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
			.Texture2D = { .MipSlice = 0 },
		};

		Check(device->CreateUnorderedAccessView(handle.Get(), &uavDesc, uav.AddressOf()));
	}
}

}
}
