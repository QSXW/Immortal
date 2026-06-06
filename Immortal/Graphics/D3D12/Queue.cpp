#include "Queue.h"
#include "Device.h"
#include "Fence.h"
#include "CommandList.h"
#include "Swapchain.h"
#include "Event.h"
#include "CommandBuffer.h"

namespace Immortal
{
namespace D3D12
{

Queue::Queue(Device *device, const D3D12_COMMAND_QUEUE_DESC &desc)
{
    Check(device->Create(&desc, &handle));
    handle->SetName(L"Command Queue");

	gpuEvent = new GPUEvent{ device };
}

Queue::~Queue()
{

}

Anonymous Queue::GetBackendHandle() const
{
	return (void *)handle.Get();
}

void Queue::WaitIdle(uint32_t timeout)
{
	handle->Signal(gpuEvent->GetFence(), gpuEvent->GetIncrement());
	gpuEvent->Wait(timeout);
}

void Queue::Wait(SuperGPUEvent *_pEvent)
{
	if (!_pEvent)
	{
		return;
	}
	GPUEvent *pEvent = InterpretAs<GPUEvent>(_pEvent);
	handle->Wait(pEvent->GetFence(), pEvent->GetValue());
}

void Queue::Signal(SuperGPUEvent *_pEvent)
{
	GPUEvent *pEvent = InterpretAs<GPUEvent>(_pEvent);
	handle->Signal(pEvent->GetFence(), pEvent->GetIncrement());
}

void Queue::Submit(SuperCommandBuffer **ppCommandBuffer, size_t count, SuperGPUEvent **ppSignalEvents, uint32_t eventCount, SuperSwapchain */*_swapchain*/)
{
	std::vector<ID3D12CommandList *> commandLists;
	commandLists.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
		CommandBuffer *commandBuffer = InterpretAs<CommandBuffer>(ppCommandBuffer[i]);
		CommandList *commandList = commandBuffer->GetCommandList();
		commandList->SetState(CommandList::State::Pending);
		commandLists.emplace_back(*commandList);
    }

    handle->ExecuteCommandLists(count, commandLists.data());

	for (size_t i = 0; i < eventCount; i++)
	{
		GPUEvent *pEvent = InterpretAs<GPUEvent>(ppSignalEvents[i]);
		handle->Signal(pEvent->GetFence(), pEvent->GetIncrement());
	}
}

void Queue::Present(SuperSwapchain *_swapchain, SuperGPUEvent **_ppSignalEvent, uint32_t eventCount)
{
	Swapchain *swapchain = InterpretAs<Swapchain>(_swapchain);
	swapchain->Present();

	GPUEvent **ppSignalEvent = (GPUEvent **)_ppSignalEvent;
	for (size_t i = 0; i < eventCount; i++)
	{
		GPUEvent *pEvent = InterpretAs<GPUEvent>(ppSignalEvent[i]);
		handle->Signal(pEvent->GetFence(), pEvent->GetIncrement());
	}
}

void Queue::ExecuteCommandLists(CommandList *pCommandList)
{
	pCommandList->SetState(CommandList::State::Pending);
	ID3D12CommandList *ppCommandList[] = { *pCommandList };
    handle->ExecuteCommandLists(1, ppCommandList);
}

HRESULT Queue::Signal(Fence *pFence, uint64_t value)
{
    return Signal(*pFence, value);
}

}
}
