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

    virtual void Map(uint32_t slot = 0) override;

    virtual void Unmap() override;

    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual void *ReadPixel(uint32_t attachmentIndex, int x, int y, Format format, int width = 1, int height = 1) override;

    virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

    virtual Attachment ColorAttachment(size_t index = 0) override;

    virtual Attachment DepthAttachment() override;

private:
    void Update();
    void Clear();

private:
    static void AttachColorTexture(uint32_t id, int samples, Texture::DataType type, uint32_t width, uint32_t height, int index);
    static void AttachDepthTexture(uint32_t id, int samples, Texture::DataType type, uint32_t attachmentType, uint32_t width, uint32_t height);

private:
    uint32_t handle;
    Framebuffer::Description desc;

    std::vector<Texture::Description> colorAttachmentDescriptions;
    Texture::Description depthAttachmentDescription{ Format::R8G8B8A8_UNORM };

    std::vector<uint32_t> colorAttachments;
    uint32_t depthAttachment;
};
}
}
