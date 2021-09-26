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
        if constexpr (typeof<T, Device>())
        {
            return *device;
        }
        if constexpr (typeof<T, CommandBuffer::ResetMode>())
        {
            return resetMode;
        }
        if constexpr (typeof<T, QueueFamilyIndex>())
        {
            return queueFamilyIndex;
        }
        if constexpr (typeof<T, ThreadIndex>())
        {
            return threadIndex;
        }
        if constexpr (typeof<T, RenderFrame>())
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

    std::vector<Unique<CommandBuffer>> primaryCommandBuffers;

    UINT32 primaryActiveCount{ 0 };

    std::vector<Unique<CommandBuffer>> secondaryCommandBuffers;

    UINT32 secondaryActiveCount{ 0 };

    CommandBuffer::ResetMode resetMode{ CommandBuffer::ResetMode::ResetPool };
    
private:
    VkResult ResetCommandBuffers();
};

}
}
