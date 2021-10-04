#pragma once

#include "D3D12Common.h"

#include "Descriptor.h"
#include "DescriptorPool.h"
#include "Barrier.h"

namespace Immortal
{
namespace D3D12
{

class CommandAllocator;
class Device;
class CommandList
{
public:
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
    CommandList(Device *device, Type type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState = nullptr);

    ~CommandList()
    {
        IfNotNullThenRelease(handle);
    }

    void Reset(ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitalState = nullptr);

    void ResourceBarrier(Barrier *barrier, UINT num = 1)
    {
        handle->ResourceBarrier(
            num,
            barrier
            );
    }

    ID3D12GraphicsCommandList *Handle()
    {
        return handle;
    }

    ID3D12GraphicsCommandList **AddressOf()
    {
        return &handle;
    }

    void Close()
    {
        Check(handle->Close());
    }

    void ClearRenderTargetView(Descriptor &descriptor, const float *clearColor, UINT numRects = 0, const D3D12_RECT *pRects = nullptr)
    {
        handle->ClearRenderTargetView(
            descriptor,
            clearColor,
            numRects,
            pRects
            );
    }

    void OMSetRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,
                            UINT                               numRenderTargetDescriptors,
                            bool                               RTsSingleHandleToDescriptorRange,
                            const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor = nullptr)
    {
        handle->OMSetRenderTargets(
            numRenderTargetDescriptors,
            pRenderTargetDescriptors,
            RTsSingleHandleToDescriptorRange,
            pDepthStencilDescriptor
            );
    }

    void SetDescriptorHeaps(DescriptorPool *descriptroHeap, UINT num)
    {
        handle->SetDescriptorHeaps(
            num,
            descriptroHeap->AddressOf()
            );
    }

private:
    ID3D12GraphicsCommandList *handle;
};

}
}
