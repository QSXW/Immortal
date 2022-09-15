#include "Queue.h"
#include "Device.h"
#include "Fence.h"
#include "CommandList.h"

namespace Immortal
{
namespace D3D12
{

Queue::Queue(Device *device, const D3D12_COMMAND_QUEUE_DESC &desc)
{
    Check(device->Create(&desc, &handle));
    handle->SetName(L"Command Queue");
}

Queue::~Queue()
{

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
