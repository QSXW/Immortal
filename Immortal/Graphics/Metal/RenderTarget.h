#pragma once

#include "Core.h"
#include "Graphics/RenderTarget.h"

#include "Common.h"
#include "Metal/MTLTexture.hpp"
#include "Texture.h"

namespace Immortal
{
namespace Metal
{

class Device;
class Texture;
class IMMORTAL_API RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;
    METAL_SWAPPABLE(RenderTarget)

public:
    RenderTarget(Device *device = nullptr);

    virtual ~RenderTarget() override;

    virtual void Resize(uint32_t width, uint32_t height) override;

	virtual SuperTexture *GetColorAttachment(uint32_t index) override;

	virtual SuperTexture *GetDepthAttachment() override;

public:
    void SetColorAttachment(MTL::Texture *texture);

public:
    const std::vector<Texture> &InternalGetColorAttachments() const
    {
		return attachments.color;
    }

    const Texture &InternalGetDepthAttachment() const
    {
		return attachments.depth;
    }

    uint32_t GetWidth() const
    {
		return attachments.color[0].GetWidth();
    }

    uint32_t GetHeight() const
    {
		return attachments.color[0].GetHeight();
    }

    void Swap(RenderTarget &other)
    {
        std::swap(attachments, other.attachments);
    }

protected:
    struct
    {
        std::vector<Texture> color;

	    Texture depth;
    } attachments;
};

}
}
