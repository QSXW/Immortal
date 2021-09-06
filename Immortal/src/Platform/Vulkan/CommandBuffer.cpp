#pragma once

#include "vkcommon.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

CommandBuffer::CommandBuffer(CommandPool *cmdPool, Level level) :
    commandPool{ cmdPool },
    level{ level }
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = cmdPool->Get<VkCommandPool>();
    allocInfo.level       = ncast<VkCommandBufferLevel>(level);

    Check(vkAllocateCommandBuffers(cmdPool->Get<Device>().Get<VkDevice>(), &allocInfo, &handle));
}

CommandBuffer::~CommandBuffer()
{
    if (handle != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(commandPool->Get<Device>().Get<VkDevice>(), commandPool->Get<VkCommandPool>(), 1, &handle);
    }
}

VkResult CommandBuffer::reset(ResetMode resetMode)
{
    VkResult result = VK_SUCCESS;

    SLASSERT(resetMode == commandPool->Get<ResetMode>() &&
        "The resetMode of Command buffer must match the one used by the pool to allocated it");
    state = State::Initial;
    if (resetMode == ResetMode::ResetIndividually)
    {
        result = vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }

    return result;
}

}
}
