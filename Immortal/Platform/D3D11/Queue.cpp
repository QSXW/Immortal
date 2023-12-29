#include "Queue.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "GPUEvent.h"

namespace Immortal
{
namespace D3D11
{

Queue::Queue()
{

}

Queue::~Queue()
{

}

void Queue::WaitIdle(uint32_t timeout)
{

}

void Queue::Wait(SuperGPUEvent *pEvent)
{

}

void Queue::Signal(SuperGPUEvent *pEvent)
{

}

void Queue::Submit(SuperCommandBuffer **_ppCommandBuffer, size_t count, SuperGPUEvent **_ppSignalEvents, uint32_t eventCount, SuperSwapchain * /*swapchain*/)
{

}

void Queue::Present(SuperSwapchain *_swapchain, SuperGPUEvent **ppSignalEvents, uint32_t eventCount)
{
	Swapchain *swapchain = InterpretAs<Swapchain>(_swapchain);
	swapchain->Present();
}

}
}
