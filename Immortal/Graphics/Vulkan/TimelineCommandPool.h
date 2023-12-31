#pragma once

#include "CommandPool.h"
#include <queue>

namespace Immortal
{
namespace Vulkan
{

class TimelineCommandBuffer;
class TimelineCommandPool : public CommandPool
{
public:
    VKCPP_SWAPPABLE(TimelineCommandPool)

public:
    TimelineCommandPool(Device *device = nullptr, uint32_t queueFamilyIndex = 0, VkCommandPoolResetFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    ~TimelineCommandPool();

    Ref<TimelineCommandBuffer> Allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    void Free(Ref<TimelineCommandBuffer> &commandBuffer);

public:
    void Swap(TimelineCommandPool &other)
    {
        CommandPool::Swap((CommandPool &)other);
        commandBuffers.swap(other.commandBuffers);
    }

protected:
    std::queue<Ref<TimelineCommandBuffer>> commandBuffers;
};

}
}
