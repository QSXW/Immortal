#include "Framebuffer.h"

#include "Device.h"
#include "RenderPass.h"
#include "ImageView.h"
#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

Framebuffer::Framebuffer() :
    handle{},
    device{}
{

}

Framebuffer::Framebuffer(Device *device, VkRenderPass renderPass, const LightArray<VkImageView> &views, const VkExtent2D &extent) :
    device{ device }
{
    VkFramebufferCreateInfo createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext           = nullptr;
    createInfo.attachmentCount = U32(views.size());
    createInfo.pAttachments    = views.data();
    createInfo.renderPass      = renderPass;
    createInfo.width           = extent.width;
    createInfo.height          = extent.height;
    createInfo.layers          = 1;

    Check(device->Create(&createInfo, &handle));
}

Framebuffer::Framebuffer(Framebuffer &&other) :
    Framebuffer{}
{
    other.Swap(*this);
}

Framebuffer &Framebuffer::operator =(Framebuffer &&other)
{
    Framebuffer(std::move(other)).Swap(*this);
    return *this;
}

void Framebuffer::Swap(Framebuffer &other)
{
    std::swap(device, other.device);
    std::swap(handle, other.handle);
}

Framebuffer::~Framebuffer()
{
    if (device)
    {
        device->DestroyAsync(handle);
    }
}

}
}
