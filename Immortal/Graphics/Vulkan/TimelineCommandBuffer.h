#pragma once

#include "CommandBuffer.h"
#include "Semaphore.h"

namespace Immortal
{
namespace Vulkan
{

class TimelineCommandBuffer : public CommandBuffer
{
public:
    TimelineCommandBuffer(CommandPool *commandPool = nullptr, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    ~TimelineCommandBuffer();

    bool IsReady() const;

public:
    TimelineSemaphore *GetTimelineSemaphore()
    {
        return &semaphore;
    }

public:
    TimelineSemaphore semaphore;
};

}
}
