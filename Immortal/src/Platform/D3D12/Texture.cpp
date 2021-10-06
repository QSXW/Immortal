#include "Texture.h"

#include "Render/Frame.h"
#include "Device.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

Texture::Texture(SuperRenderContext *context, const std::string &filepath, bool flip)
{
    auto device = rcast<RenderContext *>(context)->Get<ID3D12Device *>();

    Frame frame{ filepath, flip };

    D3D12_HEAP_PROPERTIES props{};
    CleanObject(&props);
    props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    CleanObject(&desc);
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment          = 0;
    desc.Width              = frame.Width();
    desc.Height             = frame.Height();
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource *pTexture{ nullptr };
    Check(device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&pTexture)
        ));

    D3D12_RESOURCE_DESC uploadDesc{};
    CleanObject(&uploadDesc);

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

    ID3D12Resource *uploadBuffer{ nullptr };
    Check(device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
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
}

}
}
