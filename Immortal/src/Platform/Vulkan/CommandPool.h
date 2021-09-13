#pragma once

#include "Common.h"
#include "CommandBuffer.h"

namespace Immortal
{
namespace Vulkan
{
class Device;

class CommandPool
{
public:
    using QueueFamilyIndex = UINT32;
    using ThreadIndex      = size_t;
    using RenderFrame      = void *;
    using Handle           = VkCommandPool;

public:
    CommandPool() = default;

    CommandPool(Device                  *device,
                UINT32                   queueFamilyIndex,
                void                    *renderFrame = nullptr,
                size_t                   threadIndex = 0,
                CommandBuffer::ResetMode resetMode   = CommandBuffer::ResetMode::ResetPool);
        
    CommandPool(CommandPool &&other);

    CommandPool &operator=(CommandPool &&other);

    ~CommandPool();

    CommandBuffer &RequestBuffer(Level level);

    template <class T>
    T &Get()
    {
        if constexpr (typeof<T, Handle>())
        {
            return handle;
        }
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

private:
    Device *device{ nullptr };
        
    VkCommandPool handle{ VK_NULL_HANDLE };

    void *renderFrame{ nullptr };

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
