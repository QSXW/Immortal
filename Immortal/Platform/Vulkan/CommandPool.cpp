#include "CommandPool.h"

#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

CommandPool::CommandPool(Device *device, uint32_t queueFamilyIndex, size_t threadIndex, CommandBuffer::ResetMode resetMode) :
    device{ device },
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

    VkCommandPoolCreateInfo createInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = flags,
        .queueFamilyIndex = queueFamilyIndex,
    };

    device->Create(&createInfo, &handle);
}

CommandPool::CommandPool(CommandPool &&other)
{
    __Shift(other);
}

CommandPool &CommandPool::operator=(CommandPool &&other)
{
    if (this != &other)
    {
        __Shift(other);
    }

    return *this;
}

void CommandPool::__Shift(CommandPool &other)
{
    device                  = other.device;
    handle                  = other.handle;
    queueFamilyIndex        = other.queueFamilyIndex;
    allocatedBuffers        = std::move(other.allocatedBuffers);
    queue                   = std::move(queue);
    threadIndex             = other.threadIndex;
    resetMode               = other.resetMode;

    other.handle            = VK_NULL_HANDLE;
    other.queueFamilyIndex  = 0;
}

CommandPool::~CommandPool()
{
    Destory();
}

void CommandPool::Destory()
{
    allocatedBuffers.clear();
    if (device)
    {
        device->DestroyAsync(handle);
    }
}

CommandBuffer *CommandPool::RequestBuffer(Level level)
{
    CommandBuffer *ret{};
    if (queue.empty())
    {
        ret = new CommandBuffer{ this, level };
        allocatedBuffers.emplace_back(ret);
    }
    else
    {
        ret = queue.front();
        queue.pop();
    }

    return ret;
}

void CommandPool::DiscardBuffer(CommandBuffer *commandBuffer)
{
    commandBuffer->Reset(resetMode);
    queue.push(commandBuffer);
}

VkResult CommandPool::__Reset()
{
    VkResult result = VK_SUCCESS;

    for (auto &cmd : allocatedBuffers)
    {
        result = cmd->Reset(resetMode);
        if (result != VK_SUCCESS)
        {
            break;
        }
    }

    return result;
}

TimelineCommandBuffer::TimelineCommandBuffer(Device *device, Queue::Type type)
{
    commandPool.Reset(new CommandPool{
        device,
        device->FindQueueByType(type, 0).Get<Queue::FamilyIndex>()
        });

    currentCommandBuffer = commandPool->RequestBuffer(Level::Primary);
    commandBuffers.reset(new LightArray<CommandBuffer*>);
}

const LightArray<CommandBuffer*> &TimelineCommandBuffer::GetCommandBuffers(const Timeline &timeline)
{
    if (currentCommandBuffer->Recording())
    {
        End();
    }

    if (!cache.empty())
    {
        auto device = commandPool->GetAddress<Device>();
        for (auto it = cache.begin(); it != cache.end(); )
        {
            uint64_t completion;
            Check(device->GetCompletion(it->first.semaphore, &completion));
            if (completion >= it->first.value)
            {
                for (auto &cmd : *it->second)
                {
                    commandPool->DiscardBuffer(cmd);
                }
                it->second->clear();
                it = cache.erase(it);
            }
            else 
            {
                break;
            }
        }
    }

    auto *ret = commandBuffers.get();
    cache[timeline] = std::move(commandBuffers);
    commandBuffers.reset(new LightArray<CommandBuffer*>);

    return *ret;
}

}
}
