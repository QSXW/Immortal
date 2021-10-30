#pragma once

#include "ImmortalCore.h"
#include "Render/Framebuffer.h"

namespace Immortal
{
namespace OpenGL
{
class Framebuffer : public SuperFramebuffer
{
public:
    using Super = SuperFramebuffer;

public:
    Framebuffer(const Framebuffer::Description &descrition);

    virtual ~Framebuffer();

    virtual void Map() override;

    virtual void Unmap() override;

    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual void *ReadPixel(uint32_t attachmentIndex, int x, int y, Texture::Format format, int width = 1, int height = 1) override;

    virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

    virtual uint32_t ColorAttachmentHandle(uint32_t index = 0) const override;

    virtual uint32_t DepthAttachmentHandle(uint32_t index = 0) const override;

    virtual const Framebuffer::Description &Desc() const override
    {
        return desc;
    }

private:
    void Update();
    void Clear();

private:
    static void AttachColorTexture(uint32_t id, int samples, Texture::DataType type, uint32_t width, uint32_t height, int index);
    static void AttachDepthTexture(uint32_t id, int samples, Texture::DataType type, uint32_t attachmentType, uint32_t width, uint32_t height);

private:
    uint32_t handle;
    Framebuffer::Description desc;

    std::vector<Texture::Description> mColorAttachmentSpecifications;
    Texture::Description mDepthAttachmentSpecification{ Texture::Format::None };

    std::vector<uint32_t> mColorAttachments;
    uint32_t mDepthAttachment;
};
}
}	
