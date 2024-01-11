#pragma once

#include "Common.h"
#include "Shared/IObject.h"

#include <mutex>
#include <queue>
#include <list>

namespace Immortal
{
namespace D3D12
{

class Device;
class CommandAllocator
{
public:
    using Primitive = ID3D12CommandAllocator;
	D3D12_OPERATOR_HANDLE()

    CommandAllocator(CommandAllocator &&)                = delete;
	CommandAllocator(const CommandAllocator &)           = delete;
	CommandAllocator &operator=(CommandAllocator &&)     = delete;
	CommandAllocator operator=(const CommandAllocator &) = delete;

public:
	CommandAllocator(Device *device, D3D12_COMMAND_LIST_TYPE type);

    ~CommandAllocator();

    HRESULT Reset()
    {
		return handle->Reset();
    }
};

class CommandAllocatorPool
{
public:
	CommandAllocatorPool(Device *device, D3D12_COMMAND_LIST_TYPE type);

    ~CommandAllocatorPool();

    CommandAllocator *RequestAllocator(uint64_t CompletedFenceValue);

    void DiscardAllocator(uint64_t fenceValue, CommandAllocator *allocator);

private:
    D3D12_COMMAND_LIST_TYPE type;

    Device *device;

    std::list<URef<CommandAllocator>> pool;

    std::queue<std::pair<uint64_t, CommandAllocator*>> readyAllocators;

    std::mutex mutex;
};

}
}
