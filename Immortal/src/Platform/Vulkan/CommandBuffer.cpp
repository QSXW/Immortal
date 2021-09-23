#pragma once

#include "Common.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

CommandBuffer::CommandBuffer(CommandPool *cmdPool, Level level, UINT32 count) :
    commandPool{ cmdPool },
    level{ level }
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = cmdPool->Handle();
    allocInfo.level              = ncast<VkCommandBufferLevel>(level);
    allocInfo.commandBufferCount = count;

    handles.resize(count);
    Check(vkAllocateCommandBuffers(cmdPool->Get<Device>().Handle(), &allocInfo, handles.data()));
}

CommandBuffer::~CommandBuffer()
{
    if (!handles.empty())
    {
       vkFreeCommandBuffers(commandPool->Get<Device>().Handle(), commandPool->Handle(), handles.size(), handles.data());
    }
}

VkResult CommandBuffer::Begin(VkCommandBufferUsageFlags flags, size_t index, CommandBuffer *primaryCommandBuffer)
{
    if (level == Level::Secondary)
    {
        SLASSERT(primaryCommandBuffer && "A primary command buffer pointer must be provided when calling begin from a second one");
    }
    
    if (Recoding())
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
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    return vkBeginCommandBuffer(handles[index], &beginInfo);
}

VkResult CommandBuffer::End(size_t index)
{
    if (!Recoding())
    {
        LOG::ERR("Command buffer is not already recording, call begin");
        return VK_NOT_READY;
    }
    vkEndCommandBuffer(handles[index]);
    state = State::Executable;

    return VK_SUCCESS;
}

VkResult CommandBuffer::reset(ResetMode resetMode)
{
    VkResult result = VK_SUCCESS;

    SLASSERT(resetMode == commandPool->Get<ResetMode>() &&
        "The resetMode of Command buffer must match the one used by the pool to allocated it");
    state = State::Initial;
    if (resetMode == ResetMode::ResetIndividually)
    {
        result = vkResetCommandBuffer(handles[0], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }

    return result;
}

}
}
