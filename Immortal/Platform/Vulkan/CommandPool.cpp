#include "CommandPool.h"

#include "Device.h"

namespace Immortal
{
namespace Vulkan
{
namespace Helper
{
static inline VkResult ResetCommandBuffers(std::vector<std::unique_ptr<CommandBuffer>> &commandBuffers, UINT32 &activeCount, CommandBuffer::ResetMode &resetMode)
{
    VkResult result = VK_SUCCESS;

    for (auto &cmdbuf : commandBuffers)
    {
        result = cmdbuf->reset(resetMode);
    }
    activeCount = 0;

    return result;
}
}

CommandPool::CommandPool(Device *device, UINT32 queueFamilyIndex, RenderFrame *renderFrame, size_t threadIndex, CommandBuffer::ResetMode resetMode) :
    device{ device },
    renderFrame{ renderFrame },
    threadIndex{ threadIndex },
    resetMode{ resetMode }
{
    VkCommandPoolCreateFlags flags;

    if (resetMode == CommandBuffer::ResetMode::ResetIndividually ||
        resetMode == CommandBuffer::ResetMode::AlwaysAllocated)
    {
        flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }
    else
    {
        flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    }

    VkCommandPoolCreateInfo createInfo{ };
    createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueFamilyIndex;
    createInfo.flags            = flags;

    device->Create(&createInfo, &handle);
}

CommandPool::CommandPool(CommandPool &&other) :
    device{ other.device },
    handle{ other.handle },
    queueFamilyIndex{ other.queueFamilyIndex },
    primaryCommandBuffers{ std::move(other.primaryCommandBuffers) },
    primaryActiveCount{ other.primaryActiveCount },
    secondaryCommandBuffers{ std::move(other.secondaryCommandBuffers) },
    secondaryActiveCount{ other.secondaryActiveCount },
    renderFrame{ other.renderFrame },
    threadIndex{ other.threadIndex },
    resetMode{ other.resetMode }
{
    other.handle                            = VK_NULL_HANDLE;
    other.queueFamilyIndex                  = 0;
    other.primaryActiveCount                = 0;
    other.secondaryActiveCount              = 0;
}

CommandPool &CommandPool::operator=(CommandPool &&other)
{
    if (this != &other)
    {
        device                  = other.device;
        handle                  = other.handle;
        queueFamilyIndex        = other.queueFamilyIndex;
        primaryCommandBuffers   = std::move(other.primaryCommandBuffers);
        primaryActiveCount      = other.primaryActiveCount;
        secondaryCommandBuffers = std::move(other.secondaryCommandBuffers);
        secondaryActiveCount    = other.secondaryActiveCount;
        renderFrame             = other.renderFrame;
        threadIndex             = other.threadIndex;
        resetMode               = other.resetMode;

        other.handle               = VK_NULL_HANDLE;
        other.queueFamilyIndex     = 0;
        other.primaryActiveCount   = 0;
        other.secondaryActiveCount = 0;
    }
    return *this;
}

CommandPool::~CommandPool()
{
    primaryCommandBuffers.clear();
    secondaryCommandBuffers.clear();
    if (device)
    {
        device->Destroy(handle);
    }
}

CommandBuffer *CommandPool::RequestBuffer(Level level)
{
    if (level == Level::Primary)
    {
        if (primaryActiveCount < primaryCommandBuffers.size())
        {
            return primaryCommandBuffers[primaryActiveCount++].get();
        }
        primaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(this, level));
        primaryActiveCount++;

        activeCommandBuffer = primaryCommandBuffers.back().get();
        return activeCommandBuffer;
    }
    else
    {
        if (secondaryActiveCount < secondaryCommandBuffers.size())
        {
            return secondaryCommandBuffers[secondaryActiveCount++].get();
        }
        secondaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(this, level));
        secondaryActiveCount++;

        activeCommandBuffer = secondaryCommandBuffers.back().get();
        return activeCommandBuffer;
    }
}

void CommandPool::DiscardBuffer(CommandBuffer *commandBuffer)
{
    commandBuffer->reset(resetMode);
    primaryActiveCount--;
}

VkResult CommandPool::ResetCommandBuffers()
{
    VkResult result = VK_SUCCESS;

    result = Helper::ResetCommandBuffers(primaryCommandBuffers, primaryActiveCount, resetMode);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    result = Helper::ResetCommandBuffers(secondaryCommandBuffers, secondaryActiveCount, resetMode);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    return VK_ERROR_INITIALIZATION_FAILED;
}

AsynchronousCommandBuffer::AsynchronousCommandBuffer(Device *device, Queue::Type type)
{
    commandPool.reset(new CommandPool{
        device,
        device->FindQueueByType(type, 0).Get<Queue::FamilyIndex>()
        });

    for (size_t i = 0; i < SL_ARRAY_LENGTH(commandBuffers); i++)
    {
        commandBuffers[i] = commandPool->RequestBuffer(Level::Primary);
    }
}

}
}
