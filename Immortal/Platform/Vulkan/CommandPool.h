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

    VkCommandPool &Handle()
    {
        return handle;
    }

    operator VkCommandPool() const
    {
        return handle;
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
