#pragma once

#include "Render/RenderTarget.h"

#include "Common.h"
#include "Resource.h"
#include "Descriptor.h"

namespace Immortal
{
namespace D3D12
{

class Device;

class PixelBuffer : public Resource
{
public:
    void Create(const Device *device, const Description &desc, const D3D12_CLEAR_VALUE &clearValue);
};

class ColorBuffer : public PixelBuffer
{
public:
    using Super = PixelBuffer;

public:
    ColorBuffer()
    {
        srvdescriptor.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        rtvDescriptor.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        for (int i = 0; i < SL_ARRAY_LENGTH(uavDescriptor); i++)
        {
            uavDescriptor[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        }
    }

    void Create(const Device *device, const Description &desc, const D3D12_CLEAR_VALUE &clearValue);

private:
    CPUDescriptor srvdescriptor;
    CPUDescriptor rtvDescriptor;
    CPUDescriptor uavDescriptor[12];
};

class DepthBuffer : public PixelBuffer
{
public:
    using Super = PixelBuffer;

public:
    DepthBuffer()
    {
        dsvDescriptor[0].ptr     = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        dsvDescriptor[1].ptr     = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        dsvDescriptor[2].ptr     = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        dsvDescriptor[3].ptr     = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        srvDepthDescriptor.ptr   = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        srvStencilDescriptor.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

private:
    CPUDescriptor dsvDescriptor[4];
    CPUDescriptor srvDepthDescriptor;
    CPUDescriptor srvStencilDescriptor;
};

class RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;

public:
    RenderTarget();

    RenderTarget(const Device *context, const Super::Description &descrition);

    ~RenderTarget();

    virtual void Map(uint32_t slot) override;

    virtual void Unmap() override;

    virtual void Resize(UINT32 width, UINT32 height) override;

    virtual void *ReadPixel(UINT32 handle, int x, int y, Format format, int width, int height) override;

    virtual void ClearAttachment(UINT32 attachmentIndex, int value) override;

    Resource::Description SuperToBase(const Description &description, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
    {
        Resource::Description desc{};
        desc.Alignment          = 0;
        desc.Flags              = flags;
        desc.Format             = format;
        desc.DepthOrArraySize   = description.Layers;
        desc.MipLevels          = description.MipLevels;
        desc.Width              = description.Width;
        desc.Height             = U64(description.Height);
        desc.SampleDesc.Count   = description.Samples;
        desc.SampleDesc.Quality = 0;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        return desc;
    }

private:
    struct
    {
        std::vector<ColorBuffer> color;
        DepthBuffer depth;
    } attachments;
};

}
}
