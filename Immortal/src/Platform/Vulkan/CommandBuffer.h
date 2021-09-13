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

    VkResult Begin(VkCommandBufferUsageFlags flags, CommandBuffer *primaryCommandBuffer = nullptr);

    VkResult End();

    VkCommandBuffer &Handle()
    {
        return handle;
    }

    bool Recoding()
    {
        return state == State::Recording;
    }

private:
    CommandPool *commandPool{ nullptr };

    State state{ State::Initial };

    VkCommandBuffer handle{ VK_NULL_HANDLE };

    Level level{ Level::Primary };
};

}
}
