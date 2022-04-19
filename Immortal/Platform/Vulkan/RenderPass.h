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
    using Primitive = VkRenderPass;
    VKCPP_OPERATOR_HANDLE()

public:
    RenderPass(Device *device, VkFormat colorFormat, VkFormat depthFormat, bool isPresent = true);

    RenderPass(Device *device, VkRenderPassCreateInfo *pCreateInfo);

    ~RenderPass();

private:
    Device *device{ nullptr };

    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
};

}
}
