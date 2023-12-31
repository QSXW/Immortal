#include "Queue.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "GPUEvent.h"

#include <GLFW/glfw3.h>

namespace Immortal
{
namespace OpenGL
{

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
	CommandBuffer **ppCommandBuffer = (CommandBuffer **)_ppCommandBuffer;
    for (size_t i = 0; i < count; i++)
    {
		ppCommandBuffer[i]->Execute();
    }

    GPUEvent **ppSignalEvents = (GPUEvent **)_ppSignalEvents;
    for (uint32_t i = 0; i < eventCount; i++)
    {
		ppSignalEvents[i]->Signal(1);
    }
}

void Queue::Present(SuperSwapchain *_swapchain, SuperGPUEvent **ppSignalEvents, uint32_t eventCount)
{
    Swapchain *swapchain = InterpretAs<Swapchain>(_swapchain);
    swapchain->Present();
}

}
}
