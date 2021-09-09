#pragma once

#include "Common.h"
#include "Device.h"
#include "CommandPool.h"

namespace Immortal
{
namespace Vulkan
{
namespace Helper
{
static inline VkResult ResetCommandBuffers(std::vector<Unique<CommandBuffer>> &commandBuffers, UINT32 &activeCount, CommandBuffer::ResetMode &resetMode)
{
    VkResult result = VK_SUCCESS;

    for (auto &cmdbuf : commandBuffers)
    {
        result = cmdbuf->reset(resetMode);
    }
    activeCount = 0;

    return result;
}
}

CommandPool::CommandPool(Device *device, UINT32 queueFamilyIndex, void *renderFrame, size_t threadIndex, CommandBuffer::ResetMode resetMode) :
    device{ device },
    renderFrame{ renderFrame },
    threadIndex{ threadIndex },
    resetMode{ resetMode }
{
    VkCommandPoolCreateFlags flags;
        
    if (resetMode == CommandBuffer::ResetMode::ResetIndividually ||
        resetMode == CommandBuffer::ResetMode::AlwaysAllocated)
    {
        flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }
    else
    {
        flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    }

    VkCommandPoolCreateInfo createInfo{ };
    createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueFamilyIndex;
    createInfo.flags            = flags;

    Check:vkCreateCommandPool(device->Get<VkDevice>(), &createInfo, nullptr, &handle);
}

CommandPool::CommandPool(CommandPool &&other) :
    device{ other.device },
    handle{ other.handle },
    queueFamilyIndex{ other.queueFamilyIndex },
    primaryCommandBuffers{ std::move(other.primaryCommandBuffers) },
    activePrimaryCommandBufferCount{ other.activePrimaryCommandBufferCount },
    secondaryCommandBuffers{ std::move(other.secondaryCommandBuffers) },
    activeSecondaryCommandBufferCount{ other.activeSecondaryCommandBufferCount },
    renderFrame{ other.renderFrame },
    threadIndex{ other.threadIndex },
    resetMode{ other.resetMode }
{
    other.handle                            = VK_NULL_HANDLE;
    other.queueFamilyIndex                  = 0;
    other.activePrimaryCommandBufferCount   = 0;
    other.activeSecondaryCommandBufferCount = 0;
}

CommandPool &CommandPool::operator=(CommandPool &&other)
{
    if (this != &other)
    {
        device                            = other.device;
        handle                            = other.handle;
        queueFamilyIndex                  = other.queueFamilyIndex;
        primaryCommandBuffers             = std::move(other.primaryCommandBuffers);
        activePrimaryCommandBufferCount   = other.activePrimaryCommandBufferCount;
        secondaryCommandBuffers           = std::move(other.secondaryCommandBuffers);
        activeSecondaryCommandBufferCount = other.activeSecondaryCommandBufferCount;
        renderFrame                       = other.renderFrame;
        threadIndex                       = other.threadIndex;
        resetMode                         = other.resetMode;

        other.handle                            = VK_NULL_HANDLE;
        other.queueFamilyIndex                  = 0;
        other.activePrimaryCommandBufferCount   = 0;
        other.activeSecondaryCommandBufferCount = 0;
    }
    return *this;
}

CommandPool::~CommandPool()
{
    primaryCommandBuffers.clear();
    secondaryCommandBuffers.clear();
    IfNotNullThen<VkCommandPool>(vkDestroyCommandPool, device->Get<VkDevice>(), handle, nullptr);
}

VkResult CommandPool::ResetCommandBuffers()
{
    VkResult result = VK_SUCCESS;

    result = Helper::ResetCommandBuffers(primaryCommandBuffers, activePrimaryCommandBufferCount, resetMode);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    result = Helper::ResetCommandBuffers(secondaryCommandBuffers, activeSecondaryCommandBufferCount, resetMode);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    return VK_ERROR_INITIALIZATION_FAILED;
}
}
}
