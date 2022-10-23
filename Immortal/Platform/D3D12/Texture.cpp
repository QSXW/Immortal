#include "Texture.h"

#include "Render/Frame.h"
#include "Device.h"
#include "RenderContext.h"
#include "Barrier.h"
#include "Buffer.h"
#include "Vision/Common/SamplingFactor.h"

namespace Immortal
{
namespace D3D12
{

Texture::Texture(RenderContext *context, const std::string &filepath, const Description &description) :
    context{ context }
{
    Frame frame{ filepath };

    Vision::Picture picture = frame.GetPicture();
    if (!picture.Available())
    {
        return;
    }

    SetProperties(picture.desc.width, picture.desc.height, description.mipLevels);
    format = picture.desc.format;
    __Create(picture.Data());
}

Texture::Texture(RenderContext *context, uint32_t width, uint32_t height, const void *data, const Description &description) :
    context{ context },
    Super{ width, height, description.mipLevels }
{
    format = description.format;
    __Create(data);
}

Texture::~Texture()
{
    if (context)
    {
        context->RefResource(resource.Get());
    }
}

void Texture::__Create(const void *data)
{
    Device *device = context->GetAddress<Device>();

    descriptor.invisible = context->AllocateDescriptor(DescriptorHeap::Type::ShaderResourceView);

    D3D12_HEAP_PROPERTIES props = {
        .Type                 = D3D12_HEAP_TYPE_DEFAULT,
        .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        .CreationNodeMask     = 0,
        .VisibleNodeMask      = 0,
    };

    D3D12_RESOURCE_DESC resourceDesc = {
        .Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        .Alignment          = 0,
        .Width              = width,
        .Height             = height,
        .DepthOrArraySize   = 1,
        .MipLevels          = UINT16(mipLevels),
        .Format             = format,
        .SampleDesc         = { .Count = 1, .Quality = 0 },
        .Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags              = D3D12_RESOURCE_FLAG_NONE,
    };

    Check(device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        &resource
        ));

#ifdef _DEBUG
    resource->SetName(L"Texture");
#endif

    if (data)
    {
        Update(data);
    }

    if (format < Format::YUV420P)
    {
        __InitViewWithDescriptor(descriptor.invisible);
    }
}

Texture::operator uint64_t() const
{
    if (!descriptor.visible.gpu.ptr)
    {
        Texture *This = const_cast<Texture *>(this);
        This->descriptor.visible = context->AllocateShaderVisibleDescriptor();
        This->__InitViewWithDescriptor(This->descriptor.visible.cpu);
    }
    return descriptor.visible.gpu.ptr;
}

void Texture::__InitViewWithDescriptor(CPUDescriptor descriptor)
{
    auto device = context->GetAddress<Device>();
    if (format >= Format::YUV420P)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {
            .Format                  = format,
            .ViewDimension           = D3D12_UAV_DIMENSION_TEXTURE2D,
            .Texture2D               = { .MipSlice = 0, .PlaneSlice = 0 },
        };
		device->CreateView(*this, nullptr, &desc, descriptor);
    }
    else
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            .Format                  = format,
            .ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D               = { .MostDetailedMip = 0, .MipLevels = 1, .PlaneSlice = 0, .ResourceMinLODClamp = 0 },
        };
        device->CreateView(*this, &desc, descriptor);
    }
}

CPUDescriptor Texture::GetDescriptor() const
{
    return descriptor.invisible;
}

void Texture::As(DescriptorBuffer *descriptorBuffer, size_t index)
{
    auto pDescriptor = descriptorBuffer->DeRef<CPUDescriptor>(index);
    pDescriptor[0] = descriptor.invisible;
}

bool Texture::operator==(const Super &other) const
{
    return resource == dcast<const Texture &>(other).resource;
}

void Texture::Update(const void *data, uint32_t pitchX)
{
    Device *device = context->GetAddress<Device>();

    if (!pitchX)
	{
		pitchX = width;
	}

    auto formatSize = format.Size();
    uint32_t uploadPitch = SLALIGN(pitchX * formatSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    uint32_t uploadSize  = height * uploadPitch;

    URef<Buffer> uploadBuffer = new Buffer{ context, uploadSize, Buffer::Type::TransferSource };

    void *mapped;
    Check(uploadBuffer->Map(&mapped));
    auto imageData = rcast<const uint8_t *>(data);

    for (int y = 0; y < height; y++)
    {
        memcpy((void*)((uintptr_t)mapped + y * uploadPitch), imageData + y * pitchX * formatSize, width * formatSize);
    }
    uploadBuffer->Unmap();

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {
        .pResource        = *uploadBuffer,
        .Type             = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
        .PlacedFootprint  = {
            .Offset = 0,
            .Footprint = {
                .Format = format,
                .Width  = width,
                .Height = height,
                .Depth  = 1,
                .RowPitch = uploadPitch,
            },
        },
    };

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {
        .pResource        = *this,
        .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
        .SubresourceIndex = 0,
    };

    Barrier<BarrierType::Transition> barrier{
        *this,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
    };

    context->Transfer([&](CommandList *graphicsCmdList, CommandList *cmdlist) {
        cmdlist->ResourceBarrier(&barrier);
        cmdlist->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
        barrier.Swap();
        cmdlist->ResourceBarrier(&barrier);
        });
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

void Texture::CopyImage(const uint8_t *const *data, const int *pLinesize, int height, int planes)
{
    UINT offsets[8] = {};
    UINT pitches[8] = {};
    UINT heights[8] = {};

    SamplingFactor samplingFactor{format};
    size_t uploadSize = 0;
    for (int i = 0; i < planes; i++)
    {
		offsets[i] = uploadSize;
        heights[i] = height >> (i > 0 ? samplingFactor.y : 0);
        pitches[i] = SLALIGN(pLinesize[i], D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		uploadSize += pitches[i] * heights[i];
    }

    URef<Buffer> uploadBuffer = new Buffer{ context, uploadSize, Buffer::Type::TransferSource };

    {
        uint8_t *mapped = nullptr;
        Mapper mapper(uploadBuffer, &mapped);
        for (size_t i = 0; i < planes; i++)
        {
            auto linesize = pLinesize[i];
            for (size_t j = 0; j < heights[i]; j++)
            {
                memcpy(mapped, &data[i][j * linesize], linesize);
                mapped += pitches[i];
            }
        }
    }

    context->Transfer([&](CommandList *graphicsCmdList, CommandList *cmdlist) {
        Barrier<BarrierType::Transition> barrier{
            *this,
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        };
        
        cmdlist->ResourceBarrier(&barrier);

        auto formats = GetSpecifiedPlaneFormat(format);
        for (UINT i = 0; i < planes; i++)
        {
            D3D12_TEXTURE_COPY_LOCATION src = {
                .pResource = *uploadBuffer,
                .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
                .PlacedFootprint = {
                    .Offset = offsets[i],
                    .Footprint = {
                        .Format   = formats[i],
                        .Width    = UINT(width  >> (i > 0 ? samplingFactor.x : 0)),
			            .Height   = heights[i],
                        .Depth    = 1,
                        .RowPitch = pitches[i],         
                    },
                },
            };

            D3D12_TEXTURE_COPY_LOCATION dst = {
                .pResource        = *this,
                .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                .SubresourceIndex = i,
            };

            cmdlist->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        }

        barrier.Swap();
        cmdlist->ResourceBarrier(&barrier);
    });


}

void Texture::PlatformSpecifiedUpdate(Anonymous _resource, UINT subresource)
{
	ID3D12Resource *resource = Deanonymize<ID3D12Resource *>(_resource);

    context->TransferSync([this, resource, subresource](CommandList *graphicsCmdList, CommandList *cmdlist) {
        D3D12_TEXTURE_COPY_LOCATION dst = {
            .pResource = *this,
            .Type      = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            .SubresourceIndex = 0,
        };

        D3D12_TEXTURE_COPY_LOCATION src = {
            .pResource        = resource,
            .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		    .SubresourceIndex = subresource,
        };

        Barrier<BarrierType::Transition> barriers[] = {
            {
                *this,
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            },
            {
                resource,
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_SOURCE,
		        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            }
        };

        cmdlist->ResourceBarrier(barriers, SL_ARRAY_LENGTH(barriers));
		cmdlist->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        barriers[0].Swap();
        barriers[1].Swap();
        cmdlist->ResourceBarrier(barriers, SL_ARRAY_LENGTH(barriers));
    });
}

}
}
