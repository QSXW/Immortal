#include "CommandBuffer.h"

#include "CommandPool.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

CommandBuffer::CommandBuffer(CommandPool *cmdPool, Level level) :
    commandPool{ cmdPool },
    count{ 0 },
    level{ level }
{
    Check(commandPool->Allocate(&handle, 1, (VkCommandBufferLevel)level));
}

CommandBuffer::~CommandBuffer()
{
    if (handle != VK_NULL_HANDLE)
    {
        commandPool->Release(&handle, 1);
    }
}

VkResult CommandBuffer::Begin(Usage flags, CommandBuffer *primaryCommandBuffer, const VkCommandBufferInheritanceInfo *pInheritanceInfo)
{
    if (level == Level::Secondary)
    {
        SLASSERT(primaryCommandBuffer && "A primary command buffer pointer must be provided when calling begin from a second one");
    }

    if (Recording())
    {
        LOG::ERR("Command buffer is already recording, call end first then begin");
        return VK_NOT_READY;
    }
    state = State::Recording;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = (VkCommandBufferUsageFlags)(flags);
    beginInfo.pInheritanceInfo = pInheritanceInfo;

    return vkBeginCommandBuffer(handle, &beginInfo);
}

VkResult CommandBuffer::End()
{
    if (!Recording())
    {
        LOG::ERR("Command buffer is not already recording, call begin");
        return VK_NOT_READY;
    }
    state = State::Executable;

    return vkEndCommandBuffer(handle);;
}

VkResult CommandBuffer::Execute()
{
    return VK_SUCCESS;
}

VkResult CommandBuffer::Reset(ResetMode resetMode)
{
    VkResult result = VK_SUCCESS;

    SLASSERT(resetMode == commandPool->Get<ResetMode>() &&
        "The resetMode of Command buffer must match the one used by the pool to allocated it");

    count = 0;
    state = State::Initial;
    if (resetMode == ResetMode::ResetIndividually)
    {
        result = vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }

    return result;
}

}
}
