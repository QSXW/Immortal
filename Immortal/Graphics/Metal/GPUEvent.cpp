#include "GPUEvent.h"
#include "Device.h"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLEvent.hpp"
#include <_types/_uint64_t.h>

namespace Immortal
{
namespace Metal
{

GPUEvent::GPUEvent() :
	Handle<MTL::Event>{},
	device{},
	value{}
{

}

GPUEvent::GPUEvent(Device *device) :
	device{ device },
	Handle<MTL::Event>{ device->GetHandle()->newEvent() },
    value{}
{

}

GPUEvent::~GPUEvent()
{

}

void GPUEvent::Signal(uint64_t value)
{
    MTL::CommandQueue  *commandQueue  = device->GetCommandQueue(QueueType::None);
    MTL::CommandBuffer *commandBuffer = commandQueue->commandBufferWithUnretainedReferences();
	Signal(commandBuffer, GetIncrement());
	commandBuffer->commit();
    commandBuffer->autorelease();
    commandBuffer = {};
}

void GPUEvent::Wait(uint64_t value, uint64_t timeout)
{
	uint64_t completion = GetCompletionValue();
	if (completion < value)
	{
        MTL::CommandQueue  *commandQueue  = device->GetCommandQueue(QueueType::None);
        MTL::CommandBuffer *commandBuffer = commandQueue->commandBufferWithUnretainedReferences();
		Wait(commandBuffer, value);
		commandBuffer->commit();
		commandBuffer->waitUntilCompleted();
        commandBuffer->autorelease();
        commandBuffer = {};
	}
}

void GPUEvent::Wait(uint64_t timeout)
{
	Wait(value, timeout);
}

uint64_t GPUEvent::GetCompletionValue()
{
	return (uint64_t)0;
}

uint64_t GPUEvent::GetSyncPoint()
{
	return value;
}

void GPUEvent::Signal(MTL::CommandBuffer *commandBuffer, uint64_t value)
{
	commandBuffer->encodeSignalEvent(handle, value);
}

void GPUEvent::Wait(MTL::CommandBuffer *commandBuffer, uint64_t value)
{
	commandBuffer->encodeWait(handle, value);
}

}
}
