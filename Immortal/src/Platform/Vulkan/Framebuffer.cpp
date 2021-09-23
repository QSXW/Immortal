#include "impch.h"
#include "Framebuffer.h"

#include <array>
#include <vector>

#include "RenderContext.h"
#include "Device.h"
#include "RenderPass.h"

namespace Immortal
{
namespace Vulkan
{
Framebuffer::Framebuffer()
{

}

Framebuffer::Framebuffer(const Framebuffer::Description &description) :
    Super{ description },
    context{ rcast<RenderContext*>(desc.context) },
    device{ &context->Get<Device>() }
{

}

Framebuffer::Framebuffer(Device *device, RenderPass *renderPass, std::vector<ImageView> &views, VkExtent2D &extent) :
    device{ device },
    renderPass{ renderPass }
{
    std::vector<VkImageView> attachments;

    for (auto &view : views)
    {
        attachments.emplace_back(view.Handle());
    }

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.attachmentCount = U32(attachments.size());
    createInfo.pAttachments    = attachments.data();
    createInfo.renderPass      = renderPass->Handle();
    createInfo.width           = extent.width;
    createInfo.height          = extent.height;
    createInfo.layers          = 1;

    Check(vkCreateFramebuffer(device->Handle(), &createInfo, nullptr, &handle));
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

const SuperFramebuffer::Description &Framebuffer::Desc() const
{
    return Super::Desc();
}

void Framebuffer::INIT()
{

}

}
}