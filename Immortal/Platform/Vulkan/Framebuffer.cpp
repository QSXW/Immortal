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
    Handle{},
    device{}
{

}

Framebuffer::Framebuffer(Device *device, VkRenderPass renderPass, const LightArray<VkImageView> &views, const VkExtent2D &extent) :
    device{ device }
{
    VkFramebufferCreateInfo createInfo{
        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = {},
        .renderPass      = renderPass,
        .attachmentCount = uint32_t(views.size()),
        .pAttachments    = views.data(),
        .width           = extent.width,
        .height          = extent.height,
        .layers          = 1,
    };

    Check(device->Create(&createInfo, &handle));
}

Framebuffer::~Framebuffer()
{
    Release();
}

void Framebuffer::Release()
{
    if (device)
    {
        device->DestroyAsync(handle);
        device = nullptr;
        handle = VK_NULL_HANDLE;
    }
}

}
}
