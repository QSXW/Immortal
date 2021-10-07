#include "Texture.h"

#include "Render/Frame.h"
#include "Device.h"
#include "RenderContext.h"
#include "Barrier.h"

namespace Immortal
{
namespace D3D12
{

Texture::Texture(RenderContext *context, const std::string &filepath, bool flip)
{
    auto device = context->Get<ID3D12Device *>();
    auto shaderResourceViewDescriptorHeap = context->ShaderResourceViewDescritorHeap();
    auto descriptorIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    cpuDescriptorHandle = Descriptor{
        shaderResourceViewDescriptorHeap->Get<D3D12_CPU_DESCRIPTOR_HANDLE>()
    };
    cpuDescriptorHandle.Offset(descriptorIndex, descriptorIncrementSize);

    gpuDescriptorHandle = GPUDescriptor{
        shaderResourceViewDescriptorHeap->Get<D3D12_GPU_DESCRIPTOR_HANDLE>()
    };
    gpuDescriptorHandle.Offset(descriptorIndex, descriptorIncrementSize);

    Frame frame{ filepath, flip };

    D3D12_HEAP_PROPERTIES props{};
    CleanUpObject(&props);
    props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    CleanUpObject(&desc);
    ConvertType(desc, frame.Type());
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment          = 0;
    desc.Width              = frame.Width();
    desc.Height             = frame.Height();
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    Check(device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)
        ));

    if (!texture)
    {
        OutputDebugStringW(L"Faild to Create Texture");
        return;
    }

    D3D12_RESOURCE_DESC uploadDesc{};
    CleanUpObject(&uploadDesc);

    UINT uploadPitch              = (desc.Width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
    UINT uploadSize               = desc.Height * uploadPitch;
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

    auto imageData = frame.Data();
    for (int y = 0; y < desc.Height; y++)
    {
        memcpy(
            (void *)((uintptr_t)mapped + y * uploadPitch),
            imageData + y * desc.Width * 4,
            desc.Width * 4
            );
    }

    uploadBuffer->Unmap(0, &range);

     // Copy the upload resource content into the real resource
    D3D12_TEXTURE_COPY_LOCATION srcLocation{};
    srcLocation.pResource                          = uploadBuffer;
    srcLocation.Type                               = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint.Footprint.Format   = desc.Format;
    srcLocation.PlacedFootprint.Footprint.Width    = desc.Width;
    srcLocation.PlacedFootprint.Footprint.Height   = desc.Height;
    srcLocation.PlacedFootprint.Footprint.Depth    = 1;
    srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

    D3D12_TEXTURE_COPY_LOCATION dstLocation{};
    dstLocation.pResource        = texture;
    dstLocation.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    Barrier barrier{};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = texture;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    auto queue = context->Get<Queue *>();
    auto cmdlist = queue->BeginUpload();

    cmdlist->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
    cmdlist->ResourceBarrier(&barrier);

    queue->EndUpload();

    uploadBuffer->Release();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    CleanUpObject(&srvDesc);
    srvDesc.Format                    = desc.Format;
    srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    device->CreateShaderResourceView(texture, &srvDesc, cpuDescriptorHandle);
}

Texture::~Texture()
{
    IfNotNullThenRelease(texture);
}

}
}
