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
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = *cmdPool;
    allocInfo.level              = ncast<VkCommandBufferLevel>(level);
    allocInfo.commandBufferCount = 1;
    Check(vkAllocateCommandBuffers(*cmdPool->GetAddress<Device>(), &allocInfo, &handle));
}

CommandBuffer::~CommandBuffer()
{
    if (handle != VK_NULL_HANDLE)
    {
       vkFreeCommandBuffers(*commandPool->GetAddress<Device>(), *commandPool, 1, &handle);
    }
}

VkResult CommandBuffer::Begin(Usage flags, CommandBuffer *primaryCommandBuffer)
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

    // Reset =>
    // Pipeline State
    // ResourceBinding State
    // Descriptor set layout binding state
    // Stored Push Constants

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = ncast<VkCommandBufferUsageFlags>(flags);
    beginInfo.pInheritanceInfo = nullptr;

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
