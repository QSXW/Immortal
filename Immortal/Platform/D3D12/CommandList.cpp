#include "CommandList.h"

#include "CommandAllocator.h"
#include "Device.h"
#include "Fence.h"
#include "Queue.h"

namespace Immortal
{
namespace D3D12
{

CommandList::CommandList(Device *device, Type type, CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState) :
    type{ type },
    state{ State::Pending },
    pResources{}
{
    Check(device->CreateCommandList(
        D3D12_COMMAND_LIST_TYPE(type),
        *pAllocator,
        pInitialState,
        &handle
    ));
}

CommandList::CommandList(ID3D12Device *device, Type type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState) :
    type { type },
    state{ State::Pending },
    pResources{}
{
    Check(device->CreateCommandList(
        0,
	    D3D12_COMMAND_LIST_TYPE(type),
        pAllocator,
        pInitialState,
        IID_PPV_ARGS(&handle)
    ));
}

CommandList::~CommandList()
{

}

HRESULT CommandList::Reset(CommandAllocator *pAllocator)
{
	return Reset(*pAllocator);
}

CommandListDispatcher::CommandListDispatcher(Device *device, D3D12_COMMAND_LIST_TYPE type) :
    allocatorPool{ new CommandAllocatorPool{ device, type} },
    fence{ new Fence{ device, 0 } },
    fenceValue{}
{ 
    D3D12_COMMAND_QUEUE_DESC desc = {
        .Type     = type,
        .Priority = 0,
        .Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0
    };

    queue = new Queue{ device, desc };
    allocator = allocatorPool->RequestAllocator(fenceValue);
    commandList = new CommandList{ device, CommandList::Type(type), allocator };

    Check(commandList->Close());
    queue->ExecuteCommandLists(commandList);
    __InjectSignal();
}

CommandListDispatcher::~CommandListDispatcher()
{
    WaitIdle();
    __Release();
    
    if (!resources.empty())
    {
        __ReleaseResource(resources);
    }
}

void CommandListDispatcher::__InjectSignal()
{
    queue->Signal(fence, IncreaseFence());
}

void CommandListDispatcher::__Begin()
{
    allocator = allocatorPool->RequestAllocator(fence->GetCompletion());
    Check(commandList->Reset(allocator));
    commandList->SetResource(&resources);
}

void CommandListDispatcher::WaitIdle()
{
    __InjectSignal();
    uint64_t completion = fence->GetCompletion();
    if (completion < GetFenceValue())
    {
        fence->SetCompletion(GetFenceValue());
        fence->Wait();
    }
}

void CommandListDispatcher::Execute()
{
    if (commandList->IsState(CommandList::State::Reset))
    {
        Check(commandList->Close());
        queue->ExecuteCommandLists(commandList);
        __InjectSignal();

        allocatorPool->DiscardAllocator(fenceValue, allocator);
        allocator = nullptr;

        if (!resources.empty())
        {
            resourceCache.push({ fenceValue, std::move(resources) });
        }
    }

    __Release();
}

void CommandListDispatcher::__Release()
{
    uint64_t completion = fence->GetCompletion();
    while (!resourceCache.empty())
    {
        auto &pair = resourceCache.front();
        if (pair.first <= completion)
        {
            __ReleaseResource(pair.second);
            resourceCache.pop();
        }
        else
        {
            break;
        }
    }
}

void CommandListDispatcher::__ReleaseResource(const std::list<ID3D12Resource*> &resourceChain)
{
    for (auto &resource : resourceChain)
    {
        (void)resource->Release();
    }
}

}
}
