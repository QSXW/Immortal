#include "Texture.h"

#include "Render/Frame.h"
#include "Device.h"
#include "RenderContext.h"
#include "Barrier.h"

namespace Immortal
{
namespace D3D12
{

Texture::Texture(RenderContext *context, const std::string &filepath, const Description &description) :
    context{ context },
    Super{ filepath }
{
    Frame frame{ filepath };

    Vision::Picture picture = frame.GetPicture();

    Super::Update(picture.desc.width, picture.desc.height);

    format = picture.desc.format;
    InternalCreate(picture.Data());
}

Texture::Texture(RenderContext *context, uint32_t width, uint32_t height, const void *data, const Description &description) :
    context{ context },
    Super{ width, height }
{
    format = description.format;
    InternalCreate(data);
}

Texture::~Texture()
{

}

void Texture::InternalCreate(const void *data)
{
    ID3D12Device *device = *context->GetAddress<Device>();

    descriptor.visible   = RenderContext::AllocateShaderVisibleDescriptor();
    descriptor.invisible = RenderContext::AllocateDescriptor(DescriptorHeap::Type::ShaderResourceView);

    D3D12_HEAP_PROPERTIES props{};
    props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Format             = format;
    resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment          = 0;
    resourceDesc.Width              = width;
    resourceDesc.Height             = height;
    resourceDesc.DepthOrArraySize   = 1;
    resourceDesc.MipLevels          = mipLevels;
    resourceDesc.SampleDesc.Count   = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    Check(device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&resource)
        ));

    if (!resource)
    {
        OutputDebugStringW(L"Faild to Create Texture");
        return;
    }

    if (data)
    {
        Update(data);
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    CleanUpObject(&srvDesc);
    srvDesc.Format                    = resourceDesc.Format;
    srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    device->CreateShaderResourceView(resource, &srvDesc, descriptor.visible.cpu);
    device->CreateShaderResourceView(resource, &srvDesc, descriptor.invisible);
}

void Texture::As(Descriptor::Super *descriptors, size_t index)
{
    CPUDescriptor *cpuDescriptors = rcast<CPUDescriptor *>(descriptors);
    cpuDescriptors[index] = descriptor.invisible;
}

bool Texture::operator==(const Super &other) const
{
    return resource == dcast<const Texture &>(other).resource;
}

void Texture::Update(const void *data)
{
    ID3D12Device *device = *context->GetAddress<Device>();

    D3D12_RESOURCE_DESC uploadDesc{};

    uint32_t uploadPitch          = SLALIGN(width * format.Size(), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    uint32_t uploadSize           = height * uploadPitch;
    uploadDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadDesc.Alignment          = 0;
    uploadDesc.Width              = uploadSize;
    uploadDesc.Height             = 1;
    uploadDesc.DepthOrArraySize   = 1;
    uploadDesc.MipLevels          = 1;
    uploadDesc.Format             = DXGI_FORMAT_UNKNOWN;
    uploadDesc.SampleDesc.Count   = 1;
    uploadDesc.SampleDesc.Quality = 0;
    uploadDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    uploadDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES props{};
    props.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    // Create the GPU upload buffer.
    ID3D12Resource *uploadBuffer{ nullptr };
    Check(device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
        ));

    // Write pixels into the upload resource
    void *mapped{ nullptr };
    D3D12_RANGE range{ 0, uploadSize };

    Check(uploadBuffer->Map(
        0,
        &range,
        &mapped)
        );

    auto imageData = rcast<const uint8_t *>(data);
    for (int y = 0; y < height; y++)
    {
        memcpy(
            (void *)((uintptr_t)mapped + y * uploadPitch),
            imageData + y * width * 4,
            width * 4
            );
    }
    uploadBuffer->Unmap(0, &range);

     // Copy the upload resource content into the real resource
    D3D12_TEXTURE_COPY_LOCATION srcLocation{};
    srcLocation.pResource                          = uploadBuffer;
    srcLocation.Type                               = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint.Footprint.Format   = format;
    srcLocation.PlacedFootprint.Footprint.Width    = width;
    srcLocation.PlacedFootprint.Footprint.Height   = height;
    srcLocation.PlacedFootprint.Footprint.Depth    = 1;
    srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

    D3D12_TEXTURE_COPY_LOCATION dstLocation{};
    dstLocation.pResource        = resource;
    dstLocation.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    Barrier<BarrierType::Transition> barrier{
        resource,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    };

    auto queue = context->GetAddress<Queue>();
    auto cmdlist = queue->BeginUpload();

    cmdlist->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
    cmdlist->ResourceBarrier(&barrier);

    queue->EndUpload();

    uploadBuffer->Release();
}

}
}


