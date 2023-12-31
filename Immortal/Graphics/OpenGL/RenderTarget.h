#pragma once

#include "Core.h"
#include "Graphics/RenderTarget.h"
#include "Texture.h"

namespace Immortal
{
namespace OpenGL
{

class IMMORTAL_API RenderTarget : public SuperRenderTarget, public Handle
{
public:
    using Super = SuperRenderTarget;
	GLCPP_SWAPPABLE(RenderTarget)

public:
	RenderTarget();

    RenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat = {});

    virtual ~RenderTarget() override;

    virtual void Resize(uint32_t width, uint32_t height) override;

	virtual SuperTexture *GetColorAttachment(uint32_t index) override;

	virtual SuperTexture *GetDepthAttachment() override;

public:
    uint32_t GetWidth() const
    {
		return width;
    }

    uint32_t GetHeight() const
    {
		return height;
    }

    void SetWidth(uint32_t value)
    {
		width = value;
    }

    void SetHeight(uint32_t value)
    {
		height = value;
    }

    void Swap(RenderTarget &other)
    {
		Handle::Swap(other);
		std::swap(width,       other.width      );
		std::swap(height,      other.height     );
		std::swap(attachments, other.attachments);
    }

protected:
	void Construct(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat = {});

    void Release();

protected:
	uint32_t width;

    uint32_t height;

    struct
    {
		std::vector<Texture> color;
		Texture depth;
    } attachments;
};

}
}
