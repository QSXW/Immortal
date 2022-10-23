#pragma once

#include "Common.h"
#include "CommandBuffer.h"
#include "Queue.h"
#include "Timeline.h"
#include "Algorithm/LightArray.h"
#include "Interface/IObject.h"

#include <map>
#include <queue>

namespace Immortal
{
namespace Vulkan
{

class Device;
class CommandPool : public IObject
{
public:
    using QueueFamilyIndex = uint32_t;

    using Primitive = VkCommandPool;
    VKCPP_OPERATOR_HANDLE()

public:
    CommandPool() = default;

    CommandPool(Device *device, uint32_t queueFamilyIndex, CommandBuffer::ResetMode resetMode = CommandBuffer::ResetMode::ResetIndividually);
        
    CommandPool(CommandPool &&other);

    CommandPool &operator=(CommandPool &&other);

    ~CommandPool();

    CommandBuffer *RequestBuffer(Level level);

    void DiscardBuffer(CommandBuffer *commandBuffer);

    void Destory();

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<CommandBuffer::ResetMode, T>())
        {
            return resetMode;
        }
        if constexpr (IsPrimitiveOf<QueueFamilyIndex, T>())
        {
            return queueFamilyIndex;
        }
    }

    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return device;
        }
    }

    size_t size()
    {
        return allocatedBuffers.size();
    }

private:
    void __Shift(CommandPool &other);

    VkResult __Reset();

private:
    Device *device{ nullptr };

    std::vector<URef<CommandBuffer>> allocatedBuffers;

    std::queue<CommandBuffer *> queue;

    CommandBuffer::ResetMode resetMode{ CommandBuffer::ResetMode::ResetPool };

    uint32_t queueFamilyIndex{ 0 };
};

using TimelineMap = std::map<Timeline, std::unique_ptr<LightArray<CommandBuffer*>>>;

class TimelineCommandBuffer
{
public:
    enum class DiscardType
    {
        Timeline,
        All
    };

public:
    TimelineCommandBuffer() = default;

    TimelineCommandBuffer(Device *device, Queue::Type type);
    
    ~TimelineCommandBuffer();

    const LightArray<CommandBuffer *> &GetCommandBuffers(const Timeline &timeline);

public:
    CommandBuffer *GetRecordableCommandBuffer()
    {
        if (currentCommandBuffer->ReadyToSubmit())
        {
            End();
        }
        if (!currentCommandBuffer->Recording())
        {
            currentCommandBuffer->Begin();
        }

        return currentCommandBuffer;
    }

    void End()
    {
        Check(currentCommandBuffer->End());
        commandBuffers->emplace_back(currentCommandBuffer);
        currentCommandBuffer = commandPool->RequestBuffer(Level::Primary);
    }

    bool IsRecorded() const
    {
        return currentCommandBuffer->IsRecorded() || !commandBuffers->empty();
    }

private:
    VkResult __GetCompletion(VkSemaphore semaphore, uint64_t *value) const;

    template <DiscardType T>
    void __DiscardCache()
    {
        if (!cache.empty())
        {
            for (auto it = cache.begin(); it != cache.end(); )
            {
                uint64_t completion;
                __GetCompletion(it->first.semaphore, &completion);
                if (completion >= it->first.value)
                {
                    for (auto &cmd : *it->second)
                    {
                        commandPool->DiscardBuffer(cmd);
                    }
                    it->second->clear();
                    it = cache.erase(it);
                }
                else if constexpr (T == DiscardType::Timeline)
                {
                    break;
                }
            }
        }
    }

private:
    URef<CommandPool> commandPool;

    std::unique_ptr<LightArray<CommandBuffer*>> commandBuffers;

    CommandBuffer *currentCommandBuffer;    

    TimelineMap cache;
};

}
}
