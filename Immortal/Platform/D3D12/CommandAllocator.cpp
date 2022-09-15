#include "CommandAllocator.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

CommandAllocator::CommandAllocator(Device *device, D3D12_COMMAND_LIST_TYPE type)
{
	Check(device->CreateCommandAllocator(type, &handle));
}

CommandAllocator::~CommandAllocator()
{

}

CommandAllocatorPool::CommandAllocatorPool(Device *device, D3D12_COMMAND_LIST_TYPE type) :
    type{ type },
    device{ device }
{

}

CommandAllocatorPool::~CommandAllocatorPool()
{
	pool.clear();
}

CommandAllocator *CommandAllocatorPool::RequestAllocator(uint64_t CompletedFenceValue)
{
	CommandAllocator *allocator = nullptr;

	if (!readyAllocators.empty())
	{
		std::lock_guard lock{mutex};
		std::pair<uint64_t, CommandAllocator *> allocatorInfo = readyAllocators.front();

		if (allocatorInfo.first <= CompletedFenceValue)
		{
			allocator = allocatorInfo.second;
			readyAllocators.pop();
		}
	}

	if (!allocator)
	{
		allocator = new CommandAllocator{ device, type };
		pool.emplace_back(allocator);
#ifdef _DEBUG
		wchar_t name[32];
		swprintf(name, 32, L"CommandAllocator %zu", pool.size());
		Check(allocator->SetName(name));
#endif
	}
	else
	{
		Check(allocator->Reset());
	}

	return allocator;
}

void CommandAllocatorPool::DiscardAllocator(uint64_t fenceValue, CommandAllocator *allocator)
{
	std::lock_guard<std::mutex> lock{mutex};
	readyAllocators.push({ fenceValue, allocator });
}

}
}
