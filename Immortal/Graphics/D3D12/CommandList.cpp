#include "CommandList.h"

#include "CommandAllocator.h"
#include "Device.h"
#include "Fence.h"
#include "Queue.h"

namespace Immortal
{
namespace D3D12
{

CommandList::CommandList() :
    type{},
    state{ State::Invalid },
    handle{}
{

}

CommandList::CommandList(Device *device, Type type, CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState) :
    type{ type },
    state{ State::Pending },
    handle{}
{
    DX_CHECK(device->CreateCommandList(
        D3D12_COMMAND_LIST_TYPE(type),
        *pAllocator,
        pInitialState,
	    handle.GetAddressOf()
    ));
}

CommandList::~CommandList()
{
	handle.Reset();
}

HRESULT CommandList::Reset(CommandAllocator *pAllocator)
{
	return Reset(*pAllocator);
}

}
}
