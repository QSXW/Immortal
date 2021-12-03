#pragma once

#include "ImmortalCore.h"
#include "Render/RenderTarget.h"

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

    virtual void Map(uint32_t slot = 0) override;

    virtual void Unmap() override;

    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual void *ReadPixel(uint32_t attachmentIndex, int x, int y, Format format, int width = 1, int height = 1) override;

    virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

    virtual Attachment ColorAttachment(size_t index = 0) override;

    virtual Attachment DepthAttachment() override;

    virtual uint64_t Descriptor() const
    {
        return ncast<uint64_t>(handle);
    }

private:
    void Update();
    void Clear();

private:
    static void AttachColorTexture(uint32_t id, int samples, Texture::DataType type, uint32_t width, uint32_t height, int index);
    static void AttachDepthTexture(uint32_t id, int samples, Texture::DataType type, uint32_t attachmentType, uint32_t width, uint32_t height);

private:
    uint32_t handle;
    RenderTarget::Description desc;

    std::vector<Texture::Description> colorAttachmentDescriptions;
    Texture::Description depthAttachmentDescription{ Format::R8G8B8A8_UNORM };

    std::vector<uint32_t> colorAttachments;
    uint32_t depthAttachment;
};
}
}
