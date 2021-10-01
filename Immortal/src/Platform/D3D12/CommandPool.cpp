#include "CommandPool.h"

#include "CommandAllocator.h"

namespace Immortal
{
namespace D3D12
{

CommandList::CommandList(Device *device, Type type, CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState)
{
    device->CreateCommandList(
        ncast<D3D12_COMMAND_LIST_TYPE>(type),
        pAllocator->Handle(),
        pInitialState,
        &handle
    );
}

void CommandList::Reset(CommandAllocator *pAllocator, ID3D12PipelineState *pInitalState)
{
    Check(handle->Reset(
        pAllocator->Handle(),
        pInitalState)
        );
}

}
}