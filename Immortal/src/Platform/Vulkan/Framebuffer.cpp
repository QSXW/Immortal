#include "impch.h"
#include "Framebuffer.h"

namespace Immortal
{
namespace Vulkan
{
    Framebuffer::Framebuffer(const Framebuffer::Specification &spec)
    {

    }

    Framebuffer::~Framebuffer()
    {

    }

    void Framebuffer::Map()
    {
    }

    void Framebuffer::Unmap()
    {
    }

    void Framebuffer::Resize(UINT32 width, UINT32 height)
    {
    }

    void *Framebuffer::ReadPixel(UINT32 attachmentIndex, int x, int y, Texture::Format format, int width, int height)
    {
        return nullptr;
    }

    void Framebuffer::ClearAttachment(UINT32 attachmentIndex, int value)
    {
    }

    UINT32 Framebuffer::ColorAttachmentRendererID(UINT32 index) const
    {
        return UINT32();
    }

    UINT32 Framebuffer::DepthAttachmentRendererID(UINT32 index) const
    {
        return UINT32();
    }

    const Framebuffer::Super::Specification &Framebuffer::GetSpecification() const
    {
        return Framebuffer::Super::Specification();
    }
}
}