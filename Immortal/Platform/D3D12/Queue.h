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

class Device;
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
    Queue(Device *device, const Description &desc);

    ~Queue();

    ID3D12CommandQueue *Handle() const
    {
        return handle;
    }

    operator ID3D12CommandQueue*() const
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

    HRESULT Signal(ID3D12Fence *fence, UINT64 value)
    {
        return handle->Signal(fence, value);
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
