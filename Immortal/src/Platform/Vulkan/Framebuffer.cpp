#include "impch.h"
#include "Framebuffer.h"

#include "Device.h"
#include "RenderPass.h"
#include "ImageView.h"

namespace Immortal
{
namespace Vulkan
{

Framebuffer::Framebuffer()
{

}

Framebuffer::Framebuffer(Device *device, const Framebuffer::Description &description) :
    device{ device },
    Super{ description }
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
    createInfo.renderPass      = *renderPass;
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

void *Framebuffer::ReadPixel(UINT32 attachmentIndex, int x, int y, Format format, int width, int height)
{
    return nullptr;
}

void Framebuffer::ClearAttachment(UINT32 attachmentIndex, int value)
{
}

void Framebuffer::INIT()
{

}

}
}
