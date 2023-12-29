#include "Texture.h"

#include "Render/Frame.h"
#include "Device.h"
#include "Barrier.h"
#include "Buffer.h"
#include "Vision/Common/SamplingFactor.h"

namespace Immortal
{
namespace D3D12
{

Texture::Texture(Device *device, Format _format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type) :
    Super{},
    NonDispatchableHandle{ device },
    descriptor{},
    uav{}
{
	SetMeta(width, height, mipLevels, arrayLayers);
	Construct(_format, width, height, mipLevels, arrayLayers, type);
}

void Texture::Construct(Format _format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type)
{
	format = _format;
    D3D12_HEAP_PROPERTIES props = {
        .Type                 = D3D12_HEAP_TYPE_DEFAULT,
        .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        .CreationNodeMask     = 0,
        .VisibleNodeMask      = 0,
    };

    if (type & TextureType::TransferSource)
    {
		SetState(D3D12_RESOURCE_STATE_COPY_SOURCE);
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
    }
    else if (type & TextureType::TransferDestination)
    {
		SetState(D3D12_RESOURCE_STATE_COPY_DEST);
		props.Type = D3D12_HEAP_TYPE_DEFAULT;
    }

    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
    if (type & TextureType::ColorAttachment)
    {
		flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    if (type & TextureType::DepthStencilAttachment)
    {
		flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    if (mipLevels > 1 || type &TextureType::Storage)
    {
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    D3D12_RESOURCE_DESC resourceDesc = {
        .Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        .Alignment          = 0,
        .Width              = width,
        .Height             = height,
        .DepthOrArraySize   = arrayLayers,
        .MipLevels          = mipLevels,
        .Format             = format,
        .SampleDesc         = { .Count = 1, .Quality = 0 },
        .Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags              = flags,
    };

    Check(device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
	    GetState(),
        nullptr,
        &resource
        ));

    ConstructShaderResourceView();

#ifdef _DEBUG
	resource->SetName(L"Texture");
#endif
}

Texture::Texture(Device *device, ID3D12Resource *resource, D3D12_RESOURCE_STATES state) :
    Super{},
    Resource{ resource, state },
    NonDispatchableHandle{ device },
    descriptor{},
    uav{}
{
	D3D12_RESOURCE_DESC desc = resource->GetDesc();
    
    format = desc.Format;
    SetMeta(desc.Width, desc.Height, desc.MipLevels, desc.DepthOrArraySize);

    ConstructShaderResourceView();
}

Texture::~Texture()
{
	device = nullptr;
}

void Texture::ConstructShaderResourceView()
{
	auto mipLevels = GetMipLevels();
	descriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, mipLevels);
    for (uint32_t i = 0; i < mipLevels; i++)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            .Format                  = format,
            .ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D               = {
                .MostDetailedMip     = i,
                .MipLevels           = i == 0 ? uint32_t(mipLevels) : 1,
                .PlaneSlice          = 0,
                .ResourceMinLODClamp = 0
            },
        };

		device->CreateShaderResourceView(*this, &desc, descriptor[i]);
    }
	if (mipLevels > 1 || (resource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
    {
		uav = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, mipLevels);
        for (uint32_t i = 0; i < mipLevels; i++)
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {
                .Format        = format,
				.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
                .Texture2D = {
                    .MipSlice   = i,
                    .PlaneSlice = 0 
                }
            };

			device->CreateUnorderedAccessView(*this, nullptr, &desc, uav[i]);
        }
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetDescriptor(uint32_t subresource)
{
	return descriptor[subresource];
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUAVDescriptor(uint32_t subresource)
{
    return uav[subresource];
}

static const DXGI_FORMAT *GetSpecifiedPlaneFormat(DXGI_FORMAT format)
{
    static DXGI_FORMAT formats[1];

    switch (format)
    {
        case DXGI_FORMAT_NV12:
        {
            static DXGI_FORMAT formats[] = {
                DXGI_FORMAT_R8_UNORM,
                DXGI_FORMAT_R8G8_UNORM,
            };
            return formats;
        }
        case DXGI_FORMAT_P010:
        {
            static DXGI_FORMAT formats[] = {
                DXGI_FORMAT_R16_UNORM,
                DXGI_FORMAT_R16G16_UNORM,
            };
            return formats;
        }
        default:
        {
            formats[0] = format;
            return formats;
        }
    }
}

}
}
