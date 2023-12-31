#include "CommandPool.h"

#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

CommandPool::CommandPool(Device *device, uint32_t queueFamilyIndex, VkCommandPoolResetFlags flags) :
    Handle{},
    device{ device },
    flags{ flags }
{
    VkCommandPoolCreateInfo createInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = flags,
        .queueFamilyIndex = queueFamilyIndex,
    };

    device->Create(&createInfo, &handle);
}

CommandPool::~CommandPool()
{
    Destory();
}

void CommandPool::Destory()
{
    if (device)
    {
        device->DestroyAsync(handle);
    }
}

VkResult CommandPool::Allocate(VkCommandBuffer *pCommandBuffer, uint32_t size, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = *this,
        .level              = level,
        .commandBufferCount = size,
    };

    return device->AllocateCommandBuffers(&allocInfo, pCommandBuffer);
}

void CommandPool::Release(VkCommandBuffer *pCommandBuffer, uint32_t size)
{
    device->FreeCommandBuffers(*this, size, pCommandBuffer);
}

}
}
