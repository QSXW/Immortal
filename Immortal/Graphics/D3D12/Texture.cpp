#include "Texture.h"

#include "Device.h"
#include "Barrier.h"
#include "Buffer.h"

namespace Immortal
{
namespace D3D12
{

Texture::Texture(Device *device, Format _format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type, uint32_t sampleCount, const ClearValue *pOptimizedClearValue) :
    Super{},
    NonDispatchableHandle{ device },
    descriptor{},
    uav{},
    descriptorHeap{},
    uavDescriptorHeap{}
{
	SetMeta(_format, width, height, mipLevels, arrayLayers);
	Construct(_format, width, height, mipLevels, arrayLayers, type, sampleCount, pOptimizedClearValue);
}

void Texture::Construct(Format _format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type, uint32_t sampleCount, const ClearValue *pOptimizedClearValue)
{
	format = _format;
    D3D12_HEAP_PROPERTIES props = {
        .Type                 = D3D12_HEAP_TYPE_DEFAULT,
        .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        .CreationNodeMask     = 0,
        .VisibleNodeMask      = 0,
    };

    D3D12_CLEAR_VALUE clearValues = {
	    .Format = format,
	    .Color = {},
	};
	D3D12_CLEAR_VALUE *pClearValues = nullptr;

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
		if (pOptimizedClearValue)
		{
			memcpy(clearValues.Color, pOptimizedClearValue->color.float32, sizeof(clearValues.Color));
		}
		pClearValues = &clearValues;
		flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    if (type & TextureType::DepthStencilAttachment)
    {
		if (pOptimizedClearValue)
		{
			clearValues.DepthStencil = {
			    .Depth   = pOptimizedClearValue->depthStencil.depth,
			    .Stencil = (UINT8)pOptimizedClearValue->depthStencil.stencil,
			};
		}
		else
		{
			clearValues.DepthStencil = {
			    .Depth   = 1.0f,
			    .Stencil = 0,
			};
		}
		pClearValues = &clearValues;
		flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    else if (mipLevels > 1 || type &TextureType::Storage)
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
        .SampleDesc         = { .Count = sampleCount, .Quality = 0 },
        .Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags              = flags,
    };

    if (sampleCount > 1)
    {
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels = {
			.Format           = resourceDesc.Format,
			.SampleCount      = resourceDesc.SampleDesc.Count,
		    .Flags            = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE,
		    .NumQualityLevels = 0,
        };

		DX_CHECK(device->Handle()->CheckFeatureSupport(
		    D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		    &qualityLevels,
		    sizeof(qualityLevels))
        );
		resourceDesc.SampleDesc.Quality = qualityLevels.NumQualityLevels > 0 ? qualityLevels.NumQualityLevels - 1 : 0;
		resourceDesc.MipLevels = _mipLevels = 1;
    }

    DX_CHECK(device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
	    GetState(),
	    pClearValues,
        &resource
        ));

    if (resourceDesc.SampleDesc.Count == 1)
    {
        if (type & TextureType::DepthStencilAttachment)
        {
            ConstructDepthShaderResourceView();
        }
        else
        {
		    ConstructShaderResourceView();
        }
    }

#ifdef _DEBUG
	std::wstring name = L"Texture_" + std::to_wstring(width) + L"x" + std::to_wstring(height);
	resource->SetName(name.c_str());
#endif
}

Texture::Texture(Device *device, const ComPtr<ID3D12Resource> &resource, D3D12_RESOURCE_STATES state) :
    Super{},
    Resource{ resource, state },
    NonDispatchableHandle{ device },
    descriptor{},
    uav{},
    descriptorHeap{},
    uavDescriptorHeap{}
{
	D3D12_RESOURCE_DESC desc = resource->GetDesc();

    format = desc.Format;
    SetMeta(Format::None, desc.Width, desc.Height, desc.MipLevels, desc.DepthOrArraySize);

    ConstructShaderResourceView();
}

Texture::~Texture()
{
	uint32_t mipLevels = GetMipLevels();
 	if (descriptor)
	{
		device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorHeap, descriptor, mipLevels);
	}
	if (uav)
	{
		device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, uavDescriptorHeap, uav, mipLevels);
	}

	resource.Reset();
}

void Texture::ConstructDepthShaderResourceView()
{
	DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN;
	switch (format)
	{
	case DXGI_FORMAT_D32_FLOAT:
		srvFormat = DXGI_FORMAT_R32_FLOAT;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case DXGI_FORMAT_D16_UNORM:
		srvFormat = DXGI_FORMAT_R16_UNORM;
		break;
	default:
		srvFormat = format;
		break;
	}

	auto mipLevels = GetMipLevels();
	uint32_t arrayLayers = GetArrayLayers();
	descriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &descriptorHeap, mipLevels);
	for (uint32_t i = 0; i < mipLevels; i++)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
		    .Format                  = srvFormat,
		    .ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D,
		    .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		    .Texture2D               = {
		        .MostDetailedMip     = i,
		        .MipLevels           = mipLevels - i,
		        .PlaneSlice          = 0,
		        .ResourceMinLODClamp = 0,
		    },
		};

		if (arrayLayers == 6)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			desc.TextureCube   = {
			    .MostDetailedMip     = i,
			    .MipLevels           = mipLevels - i,
			    .ResourceMinLODClamp = 0,
			};
		}
		else if (arrayLayers > 1)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray = {
			    .MostDetailedMip     = i,
			    .MipLevels           = mipLevels - i,
			    .FirstArraySlice     = 0,
			    .ArraySize           = arrayLayers,
			    .PlaneSlice          = 0,
			    .ResourceMinLODClamp = 0,
			};
		}

		device->CreateShaderResourceView(*this, &desc, descriptor[i]);
	}
}

void Texture::ConstructShaderResourceView()
{
	auto mipLevels = GetMipLevels();
	uint32_t arrayLayers = GetArrayLayers();
	descriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &descriptorHeap, mipLevels);
    for (uint32_t i = 0; i < mipLevels; i++)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            .Format                  = format,
            .ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D               = {
                .MostDetailedMip     = i,
                .MipLevels           = mipLevels - i,
                .PlaneSlice          = 0,
                .ResourceMinLODClamp = 0
            },
        };

        if (arrayLayers == 6)
        {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            desc.TextureCube   = {
                .MostDetailedMip     = i,
                .MipLevels           = mipLevels - i,
                .ResourceMinLODClamp = 0,
            };
        }
        else if (arrayLayers > 1)
        {
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray = {
			    .MostDetailedMip     = i,
			    .MipLevels           = mipLevels - i,
			    .FirstArraySlice     = 0,
			    .ArraySize           = arrayLayers,
			    .PlaneSlice          = 0,
			    .ResourceMinLODClamp = 0,
            };
        }

		device->CreateShaderResourceView(*this, &desc, descriptor[i]);
    }

	if (mipLevels > 1 || (resource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
    {
		uav = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &uavDescriptorHeap, mipLevels);
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
			if (arrayLayers > 1)
			{
				desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray = {
					.MipSlice        = i,
				    .FirstArraySlice = 0,
				    .ArraySize       = arrayLayers,
				    .PlaneSlice      = 0,
                };
			}
			device->CreateUnorderedAccessView(*this, nullptr, &desc, uav[i]);
        }
    }
}

void Texture::SetName(const char *name)
{
	resource->SetName(std::filesystem::path(name).wstring().c_str());
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
