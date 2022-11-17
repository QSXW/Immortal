#pragma once

#include "Common.h"
#include "Interface/IObject.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class RenderPass : public IObject
{
public:
    using Primitive = VkRenderPass;
    VKCPP_OPERATOR_HANDLE()

public:
    RenderPass();

    RenderPass(Device *device, VkFormat colorFormat, VkFormat depthFormat, bool isPresent = true);

    RenderPass(Device *device, VkRenderPassCreateInfo *pCreateInfo);

    RenderPass(RenderPass &&other);

    RenderPass &operator =(RenderPass &&other);

    ~RenderPass();

    void Swap(RenderPass &other);

    void Invalidate(VkRenderPass other);

    void Destroy();

    RenderPass(const RenderPass &other) = delete;
    RenderPass &operator = (const RenderPass &other) = delete;

protected:
    Device *device{ nullptr };
};

}
}
