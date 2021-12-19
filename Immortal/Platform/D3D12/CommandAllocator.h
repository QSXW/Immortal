#pragma once

#include "Common.h"
#include "CommandPool.h"

#include <mutex>
#include <queue>

namespace Immortal
{
namespace D3D12
{

class CommandAllocatorPool
{
public:
    CommandAllocatorPool(ID3D12Device *device, D3D12_COMMAND_LIST_TYPE type) :
        type{ type },
        device{ device }
    {

    }

    ~CommandAllocatorPool()
    {
        for (auto &p : pool)
        {
            p->Release();
        }
        pool.clear();
    }

    ID3D12CommandAllocator *RequestAllocator(uint64_t CompletedFenceValue)
    {
        std::lock_guard<std::mutex> lock{ mutex };
        ID3D12CommandAllocator *allocator{ nullptr };

        if (!readyAllocators.empty())
        {
            std::pair<uint64_t, ID3D12CommandAllocator*> &AllocatorPair = readyAllocators.front();

            if (AllocatorPair.first <= CompletedFenceValue)
            {
                allocator = AllocatorPair.second;
                Check(allocator->Reset());
                readyAllocators.pop();
            }
        }

        // If no allocator's were ready to be reused, create a new one
        if (!allocator)
        {
            Check(device->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator)));

            wchar_t name[32];
            swprintf(name, 32, L"CommandAllocator %zu", pool.size());
            allocator->SetName(name);
            pool.push_back(allocator);
        }

        return allocator;
    }

    void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator *allocator)
    {
        std::lock_guard<std::mutex> lock{ mutex };
        readyAllocators.push(std::make_pair(fenceValue, allocator));
    }

private:
    D3D12_COMMAND_LIST_TYPE type;

    ID3D12Device *device{ nullptr };

    std::vector<ID3D12CommandAllocator *> pool;

    std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> readyAllocators;

    std::mutex mutex;
};

}
}
