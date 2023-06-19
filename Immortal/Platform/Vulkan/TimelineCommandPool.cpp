#include "TimelineCommandPool.h"
#include "TimelineCommandBuffer.h"

namespace Immortal
{
namespace Vulkan
{

TimelineCommandPool::TimelineCommandPool(Device* device, uint32_t queueFamilyIndex , VkCommandPoolResetFlags flags) :
    CommandPool{ device, queueFamilyIndex, flags }
{

}

TimelineCommandPool::~TimelineCommandPool()
{
    while (!commandBuffers.empty())
    {
		auto &commandBuffer = commandBuffers.front();
		commandBuffers.pop();
		commandBuffer.Reset();
    }
}

Ref<TimelineCommandBuffer> TimelineCommandPool::Allocate(VkCommandBufferLevel level)
{
    if (!commandBuffers.empty())
    {
		Ref<TimelineCommandBuffer> commandBuffer = commandBuffers.front();
        if (commandBuffer->IsReady())
        {
            commandBuffer->Reset();
            commandBuffers.pop();
            return commandBuffer;
        }
    }

    return new TimelineCommandBuffer{ this, level };
}

void TimelineCommandPool::Free(Ref<TimelineCommandBuffer> &commandBuffer)
{
    commandBuffers.push(commandBuffer);
}

}
}
