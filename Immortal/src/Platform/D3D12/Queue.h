#pragma once

#include "D3D12Common.h"

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

    enum class Type
    {
        Direct       = D3D12_COMMAND_LIST_TYPE_DIRECT,
        Bundle       = D3D12_COMMAND_LIST_TYPE_BUNDLE,
        Compute      = D3D12_COMMAND_LIST_TYPE_COMPUTE,
        Copy         = D3D12_COMMAND_LIST_TYPE_COPY,
        VideoDecode  = D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
        VideoProcess = D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS,
        VideoEncode  = D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE
    };

public:
    Queue(ComPtr<ID3D12Device> device, const Description &desc)
    {
        Check(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&handle)));
    }

    ComPtr<ID3D12CommandQueue> Handle()
    {
        return handle;
    }

private:
    ComPtr<ID3D12CommandQueue> handle{ nullptr };
};

inline auto Cast(Queue::Flag flag)
{
    return ncast<D3D12_COMMAND_QUEUE_FLAGS>(flag);
}

inline auto Cast(Queue::Type type)
{
    return ncast<D3D12_COMMAND_LIST_TYPE>(type);
}

}
}

