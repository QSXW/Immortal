#pragma once

#include "Common.h"
#include "CommandBuffer.h"

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

public:
    CommandPool() = default;

    CommandPool(Device                  *device,
                UINT32                   queueFamilyIndex,
                RenderFrame             *renderFrame = nullptr,
                size_t                   threadIndex = 0,
                CommandBuffer::ResetMode resetMode   = CommandBuffer::ResetMode::ResetPool);
        
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

    VkCommandPool &Handle()
    {
        return handle;
    }

    template <class T>
    T &Get()
    {
        if constexpr (is_same<T, Device *>())
        {
            return device;
        }
        if constexpr (is_same<T, CommandBuffer::ResetMode>())
        {
            return resetMode;
        }
        if constexpr (is_same<T, QueueFamilyIndex>())
        {
            return queueFamilyIndex;
        }
        if constexpr (is_same<T, ThreadIndex>())
        {
            return threadIndex;
        }
        if constexpr (is_same<T, RenderFrame>())
        {
            return renderFrame;
        }
    }

    size_t size()
    {
        return primaryCommandBuffers.size();
    }

private:
    Device *device{ nullptr };
        
    VkCommandPool handle{ VK_NULL_HANDLE };

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

}
}
