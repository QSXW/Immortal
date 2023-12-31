#pragma once

#include "Common.h"

#include "Descriptor.h"
#include "DescriptorHeap.h"
#include "Barrier.h"

#include <atomic>
#include <queue>
#include <list>

namespace Immortal
{
namespace D3D12
{

class Device;
class Fence;
class Queue;
class CommandAllocator;
class CommandAllocatorPool;
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

    enum class State
	{
		Executable = 1,
	    Reset,
        Pending,
        Invalid,
    };

    using Primitive = ID3D12GraphicsCommandList;
    D3D12_OPERATOR_HANDLE()
	D3D_SWAPPABLE(CommandList)

public:
	CommandList();

    CommandList(Device *device, Type type, CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState = nullptr);

    CommandList(ID3D12Device *device, Type type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState = nullptr);

    ~CommandList();

    HRESULT Reset(CommandAllocator *pAllocator);

    operator ID3D12CommandList *() const
    {
        return (ID3D12CommandList *)handle.Get();
    }

    void SetState(State other)
    {
		state = other;
    }

    bool IsState(State other)
    {
		return state == other;
    }

    template <class T>
    HRESULT QueryInterface(T *pp)
    {
        return handle->QueryInterface(__uuidof(T), (void **)pp);
    }

    HRESULT Close()
    {
		SetState(State::Executable);
        return handle->Close();
    }

    HRESULT Reset(ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitalState = nullptr)
    {
        Reset();
        return handle->Reset(pAllocator, pInitalState);
    }

    void ResourceBarrier(const D3D12_RESOURCE_BARRIER *pBarrier, UINT num = 1)
    {
        Record();
        handle->ResourceBarrier(num, pBarrier);
    }

    void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, const float *clearColor, UINT numRects = 0, const D3D12_RECT *pRects = nullptr)
    {
        Record();
        handle->ClearRenderTargetView(descriptor, clearColor, numRects, pRects);
    }

    void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil, UINT numRects = 0, const D3D12_RECT *pRects = nullptr)
    {
        Record();
        handle->ClearDepthStencilView(descriptor, clearFlags, depth, stencil, numRects, pRects);
    }

    void SetRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,
                          UINT                               numRenderTargetDescriptors,
                          bool                               RTsSingleHandleToDescriptorRange,
                          const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor = nullptr)
    {
        Record();
        handle->OMSetRenderTargets(
            numRenderTargetDescriptors,
            pRenderTargetDescriptors,
            RTsSingleHandleToDescriptorRange,
            pDepthStencilDescriptor
            );
    }

    void SetDescriptorHeaps(ID3D12DescriptorHeap *const *pDescriptroHeaps, UINT num)
    {
        Record();
        handle->SetDescriptorHeaps(num, pDescriptroHeaps);
    }

    void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION *pDst, UINT dstX, UINT dstY, UINT dstZ, const D3D12_TEXTURE_COPY_LOCATION *pSrc,  const D3D12_BOX *pSrcBox)
    {
        Record();
        handle->CopyTextureRegion(pDst, dstX, dstY, dstZ, pSrc, pSrcBox);
    }
    
    void RSSetViewports(const D3D12_VIEWPORT *pViewport, UINT num = 1)
    {
        Record();
        handle->RSSetViewports(num, pViewport);
    }

    void RSSetScissorRects(const D3D12_RECT *pRect, UINT num = 1)
    {
        Record();
        handle->RSSetScissorRects(num, pRect);
    }

    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        Record();
        handle->IASetPrimitiveTopology(primitiveTopology);
    }

    void SetPipelineState(ID3D12PipelineState *pPipelineState)
    {
        Record();
        handle->SetPipelineState(pPipelineState);
    }

    void SetGraphicsRootSignature(ID3D12RootSignature *rootSignature)
    {
        Record();
        handle->SetGraphicsRootSignature(rootSignature);
    }

    void SetComputeRootSignature(ID3D12RootSignature *rootSignature)
    {
        Record();
        handle->SetComputeRootSignature(rootSignature);
    }

    void SetGraphicsRootDescriptorTable(UINT parameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
    {
        Record();
        handle->SetGraphicsRootDescriptorTable(parameterIndex, baseDescriptor);
    }

    void SetComputeRootDescriptorTable(UINT parameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
    {
        Record();
        handle->SetComputeRootDescriptorTable(parameterIndex, baseDescriptor);
    }

    void SetVertexBuffers(const D3D12_VERTEX_BUFFER_VIEW *pViews, UINT numViews = 1, UINT startSlot = 0)
    {
        Record();
        handle->IASetVertexBuffers(startSlot, numViews, pViews);
    }

    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW *pView)
    {
        Record();
        handle->IASetIndexBuffer(pView);
    }
    
    void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
    {
        Record();
        handle->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
    {
        Record();
        handle->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }

    void DispatchMesh(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
    {
		Record();
		ComPtr<ID3D12GraphicsCommandList6> commandList6;
		handle.As(&commandList6);
		commandList6->DispatchMesh(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }

    void DispatchRays(const D3D12_DISPATCH_RAYS_DESC *pDesc)
    {
		Record();
		ComPtr<ID3D12GraphicsCommandList6> commandList6;
		handle.As(&commandList6);
		commandList6->DispatchRays(pDesc);
    }

    void SetGraphicsRoot32BitConstants(UINT rootParameterIndex, UINT num32BitValuesToSet, const void *pSrcData, UINT dstOffsetIn32BitValues)
    {
        Record();
        handle->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, dstOffsetIn32BitValues);
    }
    
    void PushGraphicsConstant(uint32_t size, const void *data, uint32_t offset)
    {
        SetGraphicsRoot32BitConstants(0, SLALIGN(size, sizeof(uint32_t)) / 4, data, offset);
    }

    void SetComputeRoot32BitConstants(UINT rootParameterIndex, UINT num32BitValuesToSet, const void *pSrcData, UINT dstOffsetIn32BitValues)
    {
        Record();
        handle->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, dstOffsetIn32BitValues);
    }

    void PushComputeConstant(uint32_t size, const void *data, uint32_t offset)
    {
        SetComputeRoot32BitConstants(0, SLALIGN(size, sizeof(uint32_t)) / 4, data, offset);
    }

    void OMSetBlendFactor(const float *blendFactor)
    {
		Record();
		handle->OMSetBlendFactor(blendFactor);
    }

    void BuildRaytracingAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc, UINT numPostbuildInfoDescs, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pPostbuildInfoDescs)
    {
        Record();
        ComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
        handle.As(&pRaytracingCommandList);
        pRaytracingCommandList->BuildRaytracingAccelerationStructure(pDesc, numPostbuildInfoDescs, pPostbuildInfoDescs);
    }

protected:
    void Reset()
    {
		SetState(State::Reset);
    }

    void Record()
    {

    }

    void Swap(CommandList &other)
    {
		std::swap(handle, other.handle);
        std::swap(state,  other.state );
    }

protected:
    Type type;

	State state;
};

}
}
