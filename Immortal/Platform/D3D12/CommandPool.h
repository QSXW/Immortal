#pragma once

#include "Common.h"

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

    CommandList::CommandList(ID3D12Device *device, Type type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState = nullptr);

    ~CommandList()
    {
        IfNotNullThenRelease(handle);
    }

    void Reset(ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitalState = nullptr);

    void ResourceBarrier(const D3D12_RESOURCE_BARRIER *pBarrier, UINT num = 1)
    {
        handle->ResourceBarrier(num, pBarrier);
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

    void ClearRenderTargetView(CPUDescriptor descriptor, const float *clearColor, UINT numRects = 0, const D3D12_RECT *pRects = nullptr)
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

    void SetDescriptorHeaps(ID3D12DescriptorHeap *const *pDescriptroHeaps, UINT num)
    {
        handle->SetDescriptorHeaps(num, pDescriptroHeaps);
    }

    void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION *pDst, UINT dstX, UINT dstY, UINT dstZ, const D3D12_TEXTURE_COPY_LOCATION *pSrc,  const D3D12_BOX *pSrcBox)
    {
        handle->CopyTextureRegion(pDst, dstX, dstY, dstZ, pSrc, pSrcBox);
    }
    
    void RSSetViewports(const D3D12_VIEWPORT *pViewport, UINT num = 1)
    {
        handle->RSSetViewports(num, pViewport);
    }

    void RSSetScissorRects(const D3D12_RECT *pRect, UINT num = 1)
    {
        handle->RSSetScissorRects(num, pRect);
    }

    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        handle->IASetPrimitiveTopology(primitiveTopology);
    }

    void SetPipelineState(ID3D12PipelineState *pPipelineState)
    {
        handle->SetPipelineState(pPipelineState);
    }

    void SetGraphicsRootSignature(ID3D12RootSignature *rootSignature)
    {
        handle->SetGraphicsRootSignature(rootSignature);
    }

    void SetVertexBuffers(const D3D12_VERTEX_BUFFER_VIEW *pViews, UINT numViews = 1, UINT startSlot = 0)
    {
        handle->IASetVertexBuffers(startSlot, numViews, pViews);
    }

    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW *pView)
    {
        handle->IASetIndexBuffer(pView);
    }
    
    void DrawIndexedInstance(
        UINT indexCountPerInstance,
        UINT instanceCount,
        UINT startIndexLocation,
        INT  baseVertexLocation,
        UINT startInstanceLocation)
    {
        handle->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }


private:
    ID3D12GraphicsCommandList *handle;
};

}
}
