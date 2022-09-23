#pragma once

#include "Core.h"
#include "Render/RenderTarget.h"
#include "Texture.h"

namespace Immortal
{
namespace OpenGL
{

class RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;

public:
    RenderTarget(const RenderTarget::Description &descrition);

    virtual ~RenderTarget();

    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual void *ReadPixel(uint32_t index, int x, int y, Format format, int width = 1, int height = 1);

    virtual Attachment ColorAttachment(size_t index = 0) override;

    virtual Attachment DepthAttachment() override;

    virtual operator uint64_t() const override
    {
        return colorAttachments[0];
    }
 
    void Activate();

	void Deactivate();

    operator GLint() const
    {
        return handle;
    }

private:
    void Update();
    void Clear();

private:
    static void AttachColorTexture(uint32_t id, int samples, Texture::DataType type, uint32_t width, uint32_t height, int index);
    static void AttachDepthTexture(uint32_t id, int samples, Texture::DataType type, uint32_t attachmentType, uint32_t width, uint32_t height);

private:
    uint32_t handle;

    std::vector<Texture::Description> colorAttachmentDescriptions;
    Texture::Description depthAttachmentDescription{ Format::RGBA8 };

    std::vector<uint32_t> colorAttachments;
    uint32_t depthAttachment;
};
}
}
