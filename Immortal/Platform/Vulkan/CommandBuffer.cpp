#include "CommandBuffer.h"

#include "CommandPool.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

CommandBuffer::CommandBuffer(CommandPool *commandPool, VkCommandBufferLevel level) :
    Handle{},
    commandPool{ commandPool },
    count{ 0 },
    state{ State::Initial },
    level{ level }
{
    Check(commandPool->Allocate(&handle, 1, level));
}

CommandBuffer::~CommandBuffer()
{
	Destroy(commandPool);
}

void CommandBuffer::Destroy(CommandPool *commandPool)
{
    if (handle != VK_NULL_HANDLE)
    {
        commandPool->Release(&handle, 1);
        handle = VK_NULL_HANDLE;
    }
}

VkResult CommandBuffer::Begin(Usage flags, CommandBuffer *primaryCommandBuffer, const VkCommandBufferInheritanceInfo *pInheritanceInfo)
{
    if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
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

VkResult CommandBuffer::Reset(VkCommandBufferResetFlags flags)
{
    count = 0;
    state = State::Initial;
	return ResetCommandBuffer(flags);
}

}
}
