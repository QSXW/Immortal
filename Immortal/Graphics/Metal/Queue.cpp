#include "Queue.h"
#include "Device.h"
#include "GPUEvent.h"
#include "CommandBuffer.h"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLCommandQueue.hpp"
#include "Metal/MTLDrawable.hpp"
#include "Swapchain.h"
#include "slapi.h"

namespace Immortal
{
namespace Metal
{

Queue::Queue() :
	Handle<MTL::CommandQueue>{}
{

}

Queue::Queue(Device *device, MTL::CommandQueue *queue) :
	Handle<MTL::CommandQueue>{ queue }
{
	gpuEvent = new GPUEvent{ device };
}

Queue::~Queue()
{
	gpuEvent.Reset();
}

Anonymous Queue::GetBackendHandle() const
{
	return (void *)handle;
}

void Queue::WaitIdle(uint32_t timeout)
{
	MTL::CommandBuffer *commandBuffer = handle->commandBuffer();
	gpuEvent->Wait(commandBuffer, gpuEvent->GetValue());
}

void Queue::Wait(SuperGPUEvent *_pEvent)
{
	GPUEvent *pEvent = InterpretAs<GPUEvent>(_pEvent);
	MTL::CommandBuffer *commandBuffer = handle->commandBuffer();
	pEvent->Wait(commandBuffer, pEvent->GetValue());
	commandBuffer->commit();
	commandBuffer->autorelease();
}

void Queue::Signal(SuperGPUEvent *_pEvent)
{
	GPUEvent *pEvent = InterpretAs<GPUEvent>(_pEvent);
	MTL::CommandBuffer *commandBuffer = handle->commandBuffer();
	pEvent->Signal(commandBuffer, pEvent->GetIncrement());
	commandBuffer->commit();
	commandBuffer->autorelease();
}

void Queue::Submit(SuperCommandBuffer **ppCommandBuffer, size_t count, SuperGPUEvent **ppSignalEvents, uint32_t eventCount, SuperSwapchain */*_swapchain*/)
{
	for (size_t i = 0; i < count; i++)
	{
		CommandBuffer *commandBuffer = InterpretAs<CommandBuffer>(ppCommandBuffer[i]);
		MTL::CommandBuffer *mtlCommandBuffer = *commandBuffer;
		GPUEvent *pEvent = InterpretAs<GPUEvent>(ppSignalEvents[i]);
		pEvent->Signal(mtlCommandBuffer, pEvent->GetIncrement());
		gpuEvent->Signal(mtlCommandBuffer, gpuEvent->GetIncrement());
		commandBuffer->Commit();
	}
}

void Queue::Present(SuperSwapchain *_swapchain, SuperGPUEvent **_ppSignalEvent, uint32_t eventCount)
{
	Swapchain *swapchain = InterpretAs<Swapchain>(_swapchain);
	MTL::Drawable *drawable = swapchain->GetDrawable();

	MTL::CommandBuffer *commandBuffer = handle->commandBufferWithUnretainedReferences();
	commandBuffer->presentDrawable(drawable);
	for (uint32_t i = 0; i < eventCount; i++)
	{
		GPUEvent *pEvent = InterpretAs<GPUEvent>(_ppSignalEvent[i]);
		pEvent->Signal(commandBuffer, pEvent->GetIncrement());
	}
	commandBuffer->commit();
	commandBuffer->autorelease();
	commandBuffer = {};
}

}
}
