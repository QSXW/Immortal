#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class RenderPass
{
public:
    RenderPass(Device *device, VkFormat colorFormat, VkFormat depthFormat, bool isPresent = true);

    RenderPass(Device *device, VkRenderPassCreateInfo *pCreateInfo);

    ~RenderPass();

    VkRenderPass Handle() const
    {
        return handle;
    }

    operator VkRenderPass() const
    {
        return handle;
    }

private:
    VkRenderPass handle{ VK_NULL_HANDLE };

    Device *device{ nullptr };

    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
};

}
}
