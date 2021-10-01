#pragma once

#include "D3D12Common.h"
#include "CommandPool.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

class CommandAllocator
{
public:
    CommandAllocator(Device *device, CommandList::Type type)
    {
        device->CreateCommandAllocator(
            ncast<D3D12_COMMAND_LIST_TYPE>(type),
            &handle
        );
    }

    ~CommandAllocator()
    {
        IfNotNullThenRelease(handle);
    }

    ID3D12CommandAllocator *Handle()
    {
        return handle;
    }

private:
    ID3D12CommandAllocator *handle{ nullptr };
};

}
}
