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

    enum class Level
    {
        None      = -1,
        Primary   = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        Max       = VK_COMMAND_BUFFER_LEVEL_MAX_ENUM
    };

    VkResult reset(ResetMode reset_mode);

public:
    CommandBuffer(CommandPool *cmdPool, Level level, UINT32 count);

    ~CommandBuffer();

private:
    CommandPool *commandPool{ nullptr };

    State state{ State::Initial };

    VkCommandBuffer handle{ VK_NULL_HANDLE };

    Level level{ Level::Primary };
};

}
}
