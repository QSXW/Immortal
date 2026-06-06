#pragma once

#include "Graphics/RenderTarget.h"

#include "Common.h"
#include "Handle.h"
#include "Resource.h"
#include "Descriptor.h"
#include "Texture.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Texture;
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
            THROWIF(true, "Requested a UAV Format for a depth stencil Format.");
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

    DXGI_FORMAT GetDepthFormat(DXGI_FORMAT defaultFormat)
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

public:
    DXGI_FORMAT Format{ DXGI_FORMAT_UNKNOWN };
};

class RenderTarget : public SuperRenderTarget, public NonDispatchableHandle
{
public:
    using Super = SuperRenderTarget;

public:
    RenderTarget(Device *device = nullptr);

    ~RenderTarget();

    virtual void Resize(uint32_t width, uint32_t height) override;

	virtual SuperTexture *GetColorAttachment(uint32_t index) override;

	virtual SuperTexture *GetDepthAttachment() override;

public:
    void SetColorAttachment(uint32_t index, Ref<Texture> &texture);

	void SetDepthAttachment(Ref<Texture> &texture);

public:
    const Descriptor *GetDescriptor() const
    {
        return descriptors;
    }

    const std::vector<Ref<Texture>> &GetColorBuffers() const
    {
		return colorBuffers;
    }

    Texture *GetDepthBuffer() const
    {
		return depth;
    }

    Descriptor GetDepthBufferDescriptor() const
    {
		return depthDescriptor;
    }

    uint32_t GetWidth() const
    {
		return colorBuffers[0]->GetWidth();
    }

    uint32_t GetHeight() const
    {
		return colorBuffers[0]->GetHeight();
    }

protected:
    Descriptor descriptors[32];

    Descriptor depthDescriptor;

    std::vector<Ref<Texture>> colorBuffers;

	Ref<Texture> depth;
};

}
}
