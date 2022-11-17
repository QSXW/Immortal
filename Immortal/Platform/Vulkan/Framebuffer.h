#pragma once

#include "Common.h"
#include "RenderPass.h"
#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"
#include "Algorithm/LightArray.h"

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
    Framebuffer();

    Framebuffer(Device *device, VkRenderPass renderPass, const LightArray<VkImageView> &views, const VkExtent2D &extent);

    Framebuffer(Framebuffer &&other);

    Framebuffer &operator =(Framebuffer &&other);

    void Swap(Framebuffer &other);

    ~Framebuffer();

    Framebuffer(const Framebuffer &other) = delete;
    Framebuffer &operator = (const Framebuffer &other) = delete;

private:
    Device *device{ nullptr };
};

}
}
