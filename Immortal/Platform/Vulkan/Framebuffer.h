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
    using Primitive = VkFramebuffer;
    VKCPP_OPERATOR_HANDLE()

public:
    Framebuffer(Device *device, VkRenderPass renderPass, const std::vector<VkImageView> &views, const VkExtent2D &extent);

    ~Framebuffer();

private:
    Device *device{ nullptr };
};

}
}
