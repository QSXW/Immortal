#pragma once

#include "Common.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Fence;
class CommandList;
class Queue
{
public:
    enum class Flag
    {
        None              = D3D12_COMMAND_QUEUE_FLAG_NONE,
        DisableGPUTimeout = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT
    };

    using Primitive = ID3D12CommandQueue;
	D3D12_OPERATOR_HANDLE()

public:
	Queue(Device *device, const D3D12_COMMAND_QUEUE_DESC &desc);

    ~Queue();

    void ExecuteCommandLists(CommandList *pCommandList);

    HRESULT Signal(Fence *pFence, uint64_t value);

    void ExecuteCommandLists(ID3D12CommandList **ppCommandList, uint32_t num = 1)
    {
        handle->ExecuteCommandLists(num, ppCommandList);
    }

    HRESULT Signal(ID3D12Fence *fence, UINT64 value)
    {
        return handle->Signal(fence, value);
    }
};

class TimelineQueue
{

};

}
}
