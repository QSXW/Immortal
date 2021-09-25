#pragma once

#include "Render/Framebuffer.h"

#include "D3D12Common.h"

namespace Immortal
{

namespace D3D12
{
class Framebuffer : public SuperFramebuffer
{
public:
    using Super = SuperFramebuffer;

public:
    Framebuffer::Framebuffer();

    Framebuffer(const Super::Description &descrition);

    ~Framebuffer();

    virtual void Map() override;

    virtual void Unmap() override;

    virtual void Resize(UINT32 width, UINT32 height) override;

    virtual void *ReadPixel(UINT32 handle, int x, int y, Texture::Format format, int width, int height) override;

    virtual void ClearAttachment(UINT32 attachmentIndex, int value) override;

    virtual UINT32 ColorAttachmentHandle(UINT32 index) const override;

    virtual UINT32 DepthAttachmentHandle(UINT32 index) const override;

    virtual const Super::Description &Desc() const override;

private:
    void *handle{ nullptr };
};

}
}
