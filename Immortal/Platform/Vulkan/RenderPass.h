#pragma once

#include "Common.h"
#include "Interface/IObject.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Texture;
class RenderPass : public Handle<VkRenderPass>
{
public:
    VKCPP_SWAPPABLE(RenderPass)

public:
    RenderPass();

    RenderPass(Device *device, const std::vector<Texture> &colorAttachments, const Texture &depthAttachment, bool isPresented);

    RenderPass(Device *device, const VkRenderPassCreateInfo *pCreateInfo);

    ~RenderPass();

    void Invalidate(VkRenderPass other);

    void Release();

public:
    void Swap(RenderPass &other)
    {
        Handle::Swap(other);
        std::swap(device, other.device);
    }

protected:
    Device *device{ nullptr };
};

}
}
