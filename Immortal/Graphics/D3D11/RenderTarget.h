#pragma once

#include "Common.h"
#include "Descriptor.h"
#include "Graphics/RenderTarget.h"
#include "Algorithm/LightArray.h"
#include "Texture.h"

namespace Immortal
{
namespace D3D11
{

class IMMORTAL_API RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;
    D3D11_SWAPPABLE(RenderTarget)

public:
    RenderTarget();

    virtual ~RenderTarget() override;

	virtual void Resize(uint32_t width, uint32_t height) override;

	SuperTexture *GetColorAttachment(uint32_t index) override;

	SuperTexture *GetDepthAttachment() override;

    void AddColorAttachment(Texture &&texture);

    void ClearAttachments();

public:
    const LightArray<Texture> &GetColorAttachments() const
    {
        return colorAttachments;
    }

    const Texture &InternalGetDepthAttachment() const
    {
        return depthAttachment;
    }

    void Swap(RenderTarget &other)
    {
        std::swap(colorAttachments, other.colorAttachments);
        std::swap(depthAttachment,  other.depthAttachment );
    }

protected:
    LightArray<Texture> colorAttachments;

    Texture depthAttachment;
};

}
}
