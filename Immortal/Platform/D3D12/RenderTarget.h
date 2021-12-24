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
    DXGI_FORMAT GetUAVFormat(DXGI_FORMAT defaultFormat)
    {
        switch (defaultFormat)
        {
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM;

        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM;

        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8X8_UNORM;

        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT:
            return DXGI_FORMAT_R32_FLOAT;

#ifdef SLDEBUG
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_D16_UNORM:
            ThrowIf(true, "Requested a UAV Format for a depth stencil Format.");
            return defaultFormat;
#endif

        default:
            return defaultFormat;
        }
    }


    DXGI_FORMAT GetDSVFormat(DXGI_FORMAT defaultFormat)
    {
        switch (defaultFormat)
        {
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;

        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;

        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
            return DXGI_FORMAT_D16_UNORM;

        default:
            return defaultFormat;
        }
    }

    DXGI_FORMAT PixelBuffer::GetDepthFormat(DXGI_FORMAT defaultFormat)
    {
        switch (defaultFormat)
        {
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
            return DXGI_FORMAT_R32_FLOAT;

        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
            return DXGI_FORMAT_R16_UNORM;

        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    DXGI_FORMAT GetStencilFormat(DXGI_FORMAT defaultFormat)
    {
        switch (defaultFormat)
        {
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    void Create(const Device *device, const Description &desc, const D3D12_CLEAR_VALUE &clearValue);

public:
    DXGI_FORMAT Format{ DXGI_FORMAT_UNKNOWN };
};

class ColorBuffer : public PixelBuffer
{
public:
    using Super = PixelBuffer;

public:
    ColorBuffer()
    {
        shaderResourceViewDescriptor.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        renderTargetViewDescriptor.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        for (int i = 0; i < SL_ARRAY_LENGTH(unorderedAccessViewDescriptor); i++)
        {
            unorderedAccessViewDescriptor[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        }
    }

    void Create(Device *device, const Description &desc, const D3D12_CLEAR_VALUE &clearValue);

    template <Descriptor::Type T>
    CPUDescriptor Get() const
    {
        if constexpr (T == Descriptor::Type::ShaderResourceView)
        {
            return shaderResourceViewDescriptor;
        }
        if constexpr (T == Descriptor::Type::RenderTargetView)
        {
            return renderTargetViewDescriptor;
        }
    }

private:
    CPUDescriptor shaderResourceViewDescriptor;
    CPUDescriptor renderTargetViewDescriptor;
    CPUDescriptor unorderedAccessViewDescriptor[12];
};

class DepthBuffer : public PixelBuffer
{
public:
    using Super = PixelBuffer;

public:
    DepthBuffer()
    {
        depthStencilViewDescriptor[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        depthStencilViewDescriptor[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        depthStencilViewDescriptor[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        depthStencilViewDescriptor[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

        shaderResourceDescriptor.depth.ptr   = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        shaderResourceDescriptor.stencil.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    void Create(Device *device, const Description &desc, const D3D12_CLEAR_VALUE &clearValue);

    bool IsAvailable() const
    {
        return depthStencilViewDescriptor[0].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

private:
    CPUDescriptor depthStencilViewDescriptor[4];
    struct {
        CPUDescriptor depth;
        CPUDescriptor stencil;
    } shaderResourceDescriptor;
};

class RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;

public:
    RenderTarget();

    RenderTarget(Device *context, const Super::Description &descrition);

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

    virtual operator uint64_t() const
    {
        return 0;
    }

    operator CPUDescriptor&() const
    {
        return attachments.color[0].Get<Descriptor::Type::RenderTargetView>();
    }

    const std::vector<ColorBuffer> &GetColorBuffers() const
    {
        return attachments.color;
    }

    const DepthBuffer &GetDepthBuffer() const
    {
        return attachments.depth;
    }

    bool IsDepth() const
    {
        return attachments.depth.IsAvailable();
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
