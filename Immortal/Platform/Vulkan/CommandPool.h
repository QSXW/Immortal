#pragma once

#include "Common.h"
#include "CommandBuffer.h"
#include "Queue.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class RenderFrame;
class CommandPool
{
public:
    using QueueFamilyIndex = UINT32;

    using Primitive = VkCommandPool;
    VKCPP_OPERATOR_HANDLE()

public:
    CommandPool() = default;

    CommandPool(Device                  *device,
                UINT32                   queueFamilyIndex,
                RenderFrame             *renderFrame = nullptr,
                size_t                   threadIndex = 0,
                CommandBuffer::ResetMode resetMode   = CommandBuffer::ResetMode::ResetIndividually);
        
    CommandPool(CommandPool &&other);

    CommandPool &operator=(CommandPool &&other);

    ~CommandPool();

    CommandBuffer *RequestBuffer(Level level);

    void DiscardBuffer(CommandBuffer *commandBuffer);

    void DestoryAll()
    {
        primaryCommandBuffers.clear();
        primaryActiveCount   = 0;

        secondaryCommandBuffers.clear();
        secondaryActiveCount = 0;
    }

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
        if constexpr (IsPrimitiveOf<RenderFrame, T>())
        {
            return renderFrame;
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
        return primaryCommandBuffers.size();
    }

private:
    Device *device{ nullptr };

    RenderFrame *renderFrame{ nullptr };

    size_t threadIndex;

    UINT32 queueFamilyIndex{ 0 };

    std::vector<std::unique_ptr<CommandBuffer>> primaryCommandBuffers;

    UINT32 primaryActiveCount{ 0 };

    std::vector<std::unique_ptr<CommandBuffer>> secondaryCommandBuffers;

    UINT32 secondaryActiveCount{ 0 };

    CommandBuffer::ResetMode resetMode{ CommandBuffer::ResetMode::ResetPool };

    CommandBuffer *activeCommandBuffer{ nullptr };

private:
    VkResult ResetCommandBuffers();
};

class AsynchronousCommandBuffer
{
public:
    AsynchronousCommandBuffer() = default;

    AsynchronousCommandBuffer(Device *device, Queue::Type type);
    
public:
    /* inline */
    CommandBuffer *GetCurrentCommandBuffer() const
    {
        return commandBuffers[sync];
    }

    CommandBuffer *Begin()
    {
        commandBuffers[sync]->Begin();
        return commandBuffers[sync];
    }

    CommandBuffer *End()
    {
        commandBuffers[sync]->End();
        return commandBuffers[sync];
    }

    void Sync()
    {
        SLROTATE(sync, SL_ARRAY_LENGTH(commandBuffers));
    }

private:
    std::unique_ptr<CommandPool> commandPool;

    CommandBuffer *commandBuffers[3];

    uint32_t sync = 0;
};

}
}
