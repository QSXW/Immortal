#include "GPUEvent.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

GPUEvent::GPUEvent(Device *device) :
    TimelineSemaphore{},
	device{ device }
{
	if (device)
	{
		Check(device->AllocateSemaphore((TimelineSemaphore *) &handle));
	}
}

GPUEvent::~GPUEvent()
{
	if (device)
	{
		device->Release(std::move(*(TimelineSemaphore *)&handle));
		device = nullptr;
	}
}

GPUEvent::GPUEvent(Semaphore semaphore) :
    TimelineSemaphore{},
    device{}
{
	handle = semaphore;
}

void GPUEvent::Signal(uint64_t value)
{
	(void)value;
}

void GPUEvent::Wait(uint64_t value, uint64_t timeout)
{
	auto completion = GetCompletionValue();
	if (completion < GetValue())
	{
		VkSemaphoreWaitInfo waitInfo{ 
			.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.pNext          = nullptr,
			.flags          = 0,
			.semaphoreCount = 1,
			.pSemaphores    = &handle,
		    .pValues        = &value,
		};
		Check(device->WaitSemaphores(&waitInfo, timeout));
	}
}

void GPUEvent::Wait(uint64_t timeout)
{
	Wait(value, timeout);
}

uint64_t GPUEvent::GetCompletionValue()
{
	uint64_t completion = 0;
	Check(device->GetSemaphoreCounterValue(handle, &completion));

	return completion;
}

uint64_t GPUEvent::GetSyncPoint()
{
	return GetValue();
}

}
}
