#include "TimelineCommandBuffer.h"

namespace Immortal
{
namespace Vulkan
{

TimelineCommandBuffer::TimelineCommandBuffer(CommandPool *commandPool, VkCommandBufferLevel level) :
    CommandBuffer{ commandPool, level }
{
	Device *device = commandPool->GetAddress<Device>();
	Check(device->AllocateSemaphore(&semaphore));
}

TimelineCommandBuffer::~TimelineCommandBuffer()
{
	Device *device = commandPool->GetAddress<Device>();
	device->Release(std::move(semaphore));
}

bool TimelineCommandBuffer::IsReady() const
{
    auto device = commandPool->GetAddress<Device>();
    uint64_t completion = 0;
    Check(device->GetCompletion(semaphore, &completion));

    return completion >= semaphore.value;
}

}
}
