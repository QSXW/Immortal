#pragma once

#include "Common.h"
#include "CommandPool.h"
#include "CommandAllocator.h"

#include <mutex>

namespace Immortal
{
namespace D3D12
{

struct UploadContext
{
    std::unique_ptr<CommandList> Cmdlist{nullptr};
    ID3D12CommandAllocator *Allocator{ nullptr };
};

class Queue
{
public:
    struct Description : public D3D12_COMMAND_QUEUE_DESC
    {

    };

    enum class Flag
    {
        None              = D3D12_COMMAND_QUEUE_FLAG_NONE,
        DisableGPUTimeout = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT
    };

public:
    Queue(ComPtr<ID3D12Device> device, const Description &desc) :
        desc{ desc },
        nextFenceValue{ ncast<UINT64>(desc.Type) << 56 | 1 },
        lastCompletedFenceValue{ ncast<UINT64>(desc.Type) << 56 | 1 },
        commandAllocatorPool{ device.Get(), desc.Type }
    {
        Check(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&handle)));
        handle->SetName(L"Command Queue");

        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        fence->SetName(L"Command Queue::Fence");
        fence->Signal((uint64_t)desc.Type << 56);

        fenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
        SLASSERT(fenceEventHandle != nullptr);

        uploadContext.Allocator = RequestCommandAllocator();
        uploadContext.Cmdlist   = std::make_unique<CommandList>(
            device.Get(),
            CommandList::Type::Direct,
            uploadContext.Allocator
            );
        uploadContext.Cmdlist->Close();
        Discard(fence->GetCompletedValue(), uploadContext.Allocator);
    }

    ~Queue()
    {
        IfNotNullThenRelease(handle);
        IfNotNullThenRelease(fence);
    }

    ID3D12CommandQueue *Handle()
    {
        return handle;
    }

    UINT64 ExecuteCommandLists(CommandList *commadList, UINT num = 1)
    {
        commadList->Close();

        auto ppCommandList = commadList->AddressOf();
        handle->ExecuteCommandLists(
            num,
            (ID3D12CommandList **)ppCommandList
            );

        handle->Signal(fence, nextFenceValue);

        return nextFenceValue++;
    }

    void Execute(ID3D12CommandList **ppCommandList, uint32_t num = 1)
    {
        handle->ExecuteCommandLists(num, ppCommandList);
    }

    bool IsFenceCompleted(UINT64 fenceValue)
    {
        if (fenceValue > lastCompletedFenceValue)
        {
            auto completedValue = fence->GetCompletedValue();
            LOG::INFO("CompletedValue => {0}", completedValue);
            lastCompletedFenceValue = std::max(lastCompletedFenceValue, completedValue);
        }

        return fenceValue <= lastCompletedFenceValue;
    }

    UINT64 IncrementFence()
    {
        handle->Signal(fence, nextFenceValue);
        return nextFenceValue++;
    }

    UINT64 LastCompletedFenceValue()
    {
        return lastCompletedFenceValue;
    }

    UINT64 FenceValue()
    {
        return nextFenceValue - 1;
    }

    void WaitForGpu()
    {
        WaitForFence(IncrementFence());
    }

    void Signal(ID3D12Fence *fence, UINT64 value)
    {
        Check(handle->Signal(fence, value));
    }

    void WaitForFence(UINT64 fenceValue)
    {
        if (fence->GetCompletedValue() < fenceValue)
        {
            fence->SetEventOnCompletion(fenceValue, fenceEventHandle);
            WaitForSingleObject(fenceEventHandle, INFINITE);
            lastCompletedFenceValue = fenceValue;
        }
    }

    ID3D12CommandAllocator *RequestCommandAllocator()
    {
        UINT64 completedFence = fence->GetCompletedValue();

        return commandAllocatorPool.RequestAllocator(completedFence);
    }

    void Discard(UINT64 fenceValue, ID3D12CommandAllocator *commandAllocator)
    {
        commandAllocatorPool.DiscardAllocator(fenceValue, commandAllocator);
    }

    CommandList *BeginUpload()
    {
        uploadContext.Allocator = RequestCommandAllocator();
        uploadContext.Cmdlist->Reset(uploadContext.Allocator);

        return uploadContext.Cmdlist.get();
    }

    void EndUpload()
    {
        Discard(
            ExecuteCommandLists(uploadContext.Cmdlist.get()),
            uploadContext.Allocator
            );
        WaitForGpu();
    }

private:
    ID3D12CommandQueue *handle{ nullptr };

    UINT64 nextFenceValue;

    UINT64 lastCompletedFenceValue;

    HANDLE fenceEventHandle{ nullptr };

    ID3D12Fence *fence{ nullptr };

    Description desc{};

    CommandAllocatorPool commandAllocatorPool;

    UploadContext uploadContext;
};

}
}
