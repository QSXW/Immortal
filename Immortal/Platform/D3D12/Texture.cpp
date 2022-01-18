#include "Texture.h"

#include "Render/Frame.h"
#include "Device.h"
#include "RenderContext.h"
#include "Barrier.h"

namespace Immortal
{
namespace D3D12
{

Texture::Texture(RenderContext *context, const std::string &filepath, const Description &description)
{
    Frame frame{ filepath };

    Super::Update(frame.Width(), frame.Height());

    Description desc{};
    desc.Format = frame.Desc().Format;
    InternalCreate(context, desc, frame.Data());
}

Texture::Texture(RenderContext *context, uint32_t width, uint32_t height, const void *data, const Description &description) :
    Super{ width, height }
{
    InternalCreate(context, description, data);
}

Texture::~Texture()
{

}

void Texture::InternalCreate(RenderContext *context, const Description &description, const void *data)
{
    ID3D12Device *device = *context->GetAddress<Device>();

    descriptor = context->AllocateShaderVisibleDescriptor();

    D3D12_HEAP_PROPERTIES props{};
    CleanUpObject(&props);
    props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resourceDesc{};
    CleanUpObject(&resourceDesc);
    resourceDesc.Format             = description.BaseFromat<DXGI_FORMAT>();
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

    D3D12_RESOURCE_DESC uploadDesc{};
    CleanUpObject(&uploadDesc);

    UINT uploadPitch              = (resourceDesc.Width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
    UINT uploadSize               = resourceDesc.Height * uploadPitch;
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
    for (int y = 0; y < resourceDesc.Height; y++)
    {
        memcpy(
            (void *)((uintptr_t)mapped + y * uploadPitch),
            imageData + y * resourceDesc.Width * 4,
            resourceDesc.Width * 4
            );
    }
    uploadBuffer->Unmap(0, &range);

     // Copy the upload resource content into the real resource
    D3D12_TEXTURE_COPY_LOCATION srcLocation{};
    srcLocation.pResource                          = uploadBuffer;
    srcLocation.Type                               = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint.Footprint.Format   = resourceDesc.Format;
    srcLocation.PlacedFootprint.Footprint.Width    = resourceDesc.Width;
    srcLocation.PlacedFootprint.Footprint.Height   = resourceDesc.Height;
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

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    CleanUpObject(&srvDesc);
    srvDesc.Format                    = resourceDesc.Format;
    srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = resourceDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    device->CreateShaderResourceView(resource, &srvDesc, descriptor.cpu);
}

void Texture::As(Descriptor::Super *descriptors, size_t index)
{
    GPUDescriptor *gpuDescriptors = rcast<GPUDescriptor *>(descriptors);
    gpuDescriptors[index] = descriptor.gpu;
}

}
}
