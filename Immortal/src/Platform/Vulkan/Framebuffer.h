#pragma once

#include "Common.h"
#include "RenderPass.h"
#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"

namespace Immortal
{
namespace Vulkan
{

class Framebuffer
{
public:
    Framebuffer(Device *device, VkRenderPass renderPass, const std::vector<VkImageView> &views, VkExtent2D &extent);

    ~Framebuffer();

    VkFramebuffer &Handle()
    {
        return handle;
    }

    operator VkFramebuffer&()
    {
        return handle;
    }

    operator VkFramebuffer() const
    {
        return handle;
    }

private:
    Device *device{ nullptr };

    VkFramebuffer handle{ VK_NULL_HANDLE };
};
}
}
