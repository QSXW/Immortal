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

    CommandPool(Device *device, uint32_t queueFamilyIndex, size_t threadIndex = 0, CommandBuffer::ResetMode resetMode = CommandBuffer::ResetMode::ResetIndividually);
        
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
        if constexpr (IsPrimitiveOf<ThreadIndex, T>())
        {
            return threadIndex;
        }
    }

    template <class T>
    requires std::is_same_v<Device, T>
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

    size_t threadIndex;

    std::vector<MonoRef<CommandBuffer>> allocatedBuffers;

    std::queue<CommandBuffer *> queue;

    CommandBuffer::ResetMode resetMode{ CommandBuffer::ResetMode::ResetPool };

    uint32_t queueFamilyIndex{ 0 };
};

using TimelineMap = std::map<Timeline, std::unique_ptr<LightArray<CommandBuffer*>>>;

class TimelineCommandBuffer
{
public:
    TimelineCommandBuffer() = default;

    TimelineCommandBuffer(Device *device, Queue::Type type);
    
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
    MonoRef<CommandPool> commandPool;

    std::unique_ptr<LightArray<CommandBuffer*>> commandBuffers;

    CommandBuffer *currentCommandBuffer;    

    TimelineMap cache;

    uint32_t sync = 0;
};

}
}
