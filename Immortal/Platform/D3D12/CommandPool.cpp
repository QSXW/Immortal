#include "CommandPool.h"

#include "CommandAllocator.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

CommandList::CommandList(Device *device, Type type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState)
{
    device->CreateCommandList(
        ncast<D3D12_COMMAND_LIST_TYPE>(type),
        pAllocator,
        pInitialState,
        &handle
    );
}

CommandList::CommandList(ID3D12Device *device, Type type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState)
{
    device->CreateCommandList(
        0,
        ncast<D3D12_COMMAND_LIST_TYPE>(type),
        pAllocator,
        pInitialState,
        IID_PPV_ARGS(&handle)
    );
}

void CommandList::Reset(ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitalState)
{
    Check(handle->Reset(
        pAllocator,
        pInitalState
        ));
}


}
}