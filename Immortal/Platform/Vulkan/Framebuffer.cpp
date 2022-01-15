#include "impch.h"
#include "Framebuffer.h"

#include "Device.h"
#include "RenderPass.h"
#include "ImageView.h"
#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

Framebuffer::Framebuffer(Device *device, VkRenderPass renderPass, const std::vector<VkImageView> &views, const VkExtent2D &extent) :
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

    Check(device->Create(&createInfo, nullptr, &handle));
}

Framebuffer::~Framebuffer()
{
    if (device)
    {
        device->Destroy(handle);
    }
}

}
}
