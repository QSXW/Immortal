#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class CommandPool;
class CommandBuffer
{
public:
    enum class ResetMode
    {
        ResetPool,
        ResetIndividually,
        AlwaysAllocated
    };

    enum class State
    {
        Invalid,
        Initial,
        Recording,
        Executable
    };

    enum class Usage : int
    {
        
    };

    VkResult reset(ResetMode reset_mode);

public:
    CommandBuffer(CommandPool *cmdPool, Level level, UINT32 count = 1);

    ~CommandBuffer();

    VkResult Begin(VkCommandBufferUsageFlags flags, size_t index = 0, CommandBuffer *primaryCommandBuffer = nullptr);

    VkResult End(size_t index = 0);

    VkCommandBuffer &Handle(int index = 0)
    {
        return handles[index];
    }

    bool Recoding()
    {
        return state == State::Recording;
    }

    size_t Size()
    {
        return handles.size();
    }

    VkCommandBuffer *Data()
    {
        return handles.data();
    }

private:
    CommandPool *commandPool{ nullptr };

    State state{ State::Initial };

    std::vector<VkCommandBuffer> handles{ VK_NULL_HANDLE };

    Level level{ Level::Primary };
};

}
}
