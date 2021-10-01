#pragma once

#include "D3D12Common.h"
#include "CommandPool.h"

namespace Immortal
{
namespace D3D12
{

class Queue
{
public:
    using Description = D3D12_COMMAND_QUEUE_DESC;
    
    enum class Flag
    {
        None              = D3D12_COMMAND_QUEUE_FLAG_NONE,
        DisableGPUTimeout = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT
    };

public:
    Queue(ComPtr<ID3D12Device> device, const Description &desc)
    {
        Check(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&handle)));
    }

    ~Queue()
    {
        IfNotNullThenRelease(handle);
    }

    ID3D12CommandQueue *Handle()
    {
        return handle;
    }

    void ExecuteCommandLists(CommandList *commadList, UINT num = 1)
    {
        auto ppCommandList = commadList->AddressOf();
        handle->ExecuteCommandLists(
            num,
            (ID3D12CommandList **)ppCommandList
            );
    }

private:
    ID3D12CommandQueue *handle{ nullptr };
};

}
}
