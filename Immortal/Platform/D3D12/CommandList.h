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
    };

    using Primitive = ID3D12GraphicsCommandList;
    D3D12_OPERATOR_HANDLE()

public:
    CommandList(Device *device, Type type, CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState = nullptr);

    CommandList(ID3D12Device *device, Type type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState = nullptr);

    ~CommandList();

    HRESULT Reset(CommandAllocator *pAllocator);

    operator ID3D12CommandList *() const
    {
        return (ID3D12CommandList *)handle.Get();
    }

    bool IsSuitableSubmitted()
    {
        if (type == Type::Direct)
        {
            return count == 1000;
        }
        if (type == Type::Compute)
        {
            return count = 100;
        }
        if (type == Type::Copy)
        {
            return count == 10;
        }

        return false;
    }

    void SetState(State other)
    {
		state = other;
    }

    bool IsState(State other)
    {
		return state == other;
    }

    void SetResource(std::list<ID3D12Resource *> *other)
    {
        pResources = other;
    }

    HRESULT Close()
    {
		__Close();
        return handle->Close();
    }

    HRESULT Reset(ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitalState = nullptr)
    {
        __Reset();
        return handle->Reset(pAllocator, pInitalState);
    }

    void ResourceBarrier(const D3D12_RESOURCE_BARRIER *pBarrier, UINT num = 1)
    {
        __Record();
        handle->ResourceBarrier(num, pBarrier);
    }

    void ClearRenderTargetView(CPUDescriptor descriptor, const float *clearColor, UINT numRects = 0, const D3D12_RECT *pRects = nullptr)
    {
        __Record();
        handle->ClearRenderTargetView(descriptor, clearColor, numRects, pRects);
    }

    void ClearDepthStencilView(CPUDescriptor descriptor, D3D12_CLEAR_FLAGS clearFlags, FLOAT depth, UINT8 stencil, UINT numRects = 0, const D3D12_RECT *pRects = nullptr)
    {
        __Record();
        handle->ClearDepthStencilView(descriptor, clearFlags, depth, stencil, numRects, pRects);
    }

    void SetRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,
                          UINT                               numRenderTargetDescriptors,
                          bool                               RTsSingleHandleToDescriptorRange,
                          const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor = nullptr)
    {
        __Record();
        handle->OMSetRenderTargets(
            numRenderTargetDescriptors,
            pRenderTargetDescriptors,
            RTsSingleHandleToDescriptorRange,
            pDepthStencilDescriptor
            );
    }

    void SetDescriptorHeaps(ID3D12DescriptorHeap *const *pDescriptroHeaps, UINT num)
    {
        __Record();
        handle->SetDescriptorHeaps(num, pDescriptroHeaps);
    }

    void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION *pDst, UINT dstX, UINT dstY, UINT dstZ, const D3D12_TEXTURE_COPY_LOCATION *pSrc,  const D3D12_BOX *pSrcBox)
    {
        __Record();
        if (pResources)
        {
            pSrc->pResource->AddRef();
            pResources->emplace_back(pSrc->pResource);
        }
        handle->CopyTextureRegion(pDst, dstX, dstY, dstZ, pSrc, pSrcBox);
    }
    
    void RSSetViewports(const D3D12_VIEWPORT *pViewport, UINT num = 1)
    {
        __Record();
        handle->RSSetViewports(num, pViewport);
    }

    void RSSetScissorRects(const D3D12_RECT *pRect, UINT num = 1)
    {
        __Record();
        handle->RSSetScissorRects(num, pRect);
    }

    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        __Record();
        handle->IASetPrimitiveTopology(primitiveTopology);
    }

    void SetPipelineState(ID3D12PipelineState *pPipelineState)
    {
        __Record();
        handle->SetPipelineState(pPipelineState);
    }

    void SetGraphicsRootSignature(ID3D12RootSignature *rootSignature)
    {
        __Record();
        handle->SetGraphicsRootSignature(rootSignature);
    }

    void SetComputeRootSignature(ID3D12RootSignature *rootSignature)
    {
        __Record();
        handle->SetComputeRootSignature(rootSignature);
    }

    void SetGraphicsRootDescriptorTable(UINT parameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
    {
        __Record();
        handle->SetGraphicsRootDescriptorTable(parameterIndex, baseDescriptor);
    }

    void SetComputeRootDescriptorTable(UINT parameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
    {
        __Record();
        handle->SetComputeRootDescriptorTable(parameterIndex, baseDescriptor);
    }

    void SetVertexBuffers(const D3D12_VERTEX_BUFFER_VIEW *pViews, UINT numViews = 1, UINT startSlot = 0)
    {
        __Record();
        handle->IASetVertexBuffers(startSlot, numViews, pViews);
    }

    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW *pView)
    {
        __Record();
        handle->IASetIndexBuffer(pView);
    }
    
    void DrawIndexedInstance(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT  baseVertexLocation, UINT startInstanceLocation)
    {
        __Record();
        handle->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
    {
        __Record();
        handle->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }

    void SetGraphicsRoot32BitConstants(UINT rootParameterIndex, UINT num32BitValuesToSet, const void *pSrcData, UINT dstOffsetIn32BitValues)
    {
        __Record();
        handle->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, dstOffsetIn32BitValues);
    }
    
    void PushConstant(uint32_t size, const void *data, uint32_t offset)
    {
        SetGraphicsRoot32BitConstants(0, SLALIGN(size, sizeof(uint32_t)) / 4, data, offset);
    }

    void SetComputeRoot32BitConstants(UINT rootParameterIndex, UINT num32BitValuesToSet, const void *pSrcData, UINT dstOffsetIn32BitValues)
    {
        __Record();
        handle->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, dstOffsetIn32BitValues);
    }

    void PushComputeConstant(uint32_t size, const void *data, uint32_t offset)
    {
        SetComputeRoot32BitConstants(0, SLALIGN(size, sizeof(uint32_t)) / 4, data, offset);
    }

protected:
    void __Close()
    {
		SetState(State::Executable);
    }

    void __Reset()
    {
        pResources = nullptr;
		SetState(State::Reset);
        count = 0;
    }

    void __Record()
    {
        count++;
    }

protected:
    Type type;

	State state;

    std::atomic_uint64_t count;

    std::list<ID3D12Resource *> *pResources;
};

class CommandListDispatcher
{
public:
	CommandListDispatcher(Device *device, D3D12_COMMAND_LIST_TYPE type);

    ~CommandListDispatcher();

    void WaitIdle();

    void Execute();

    CommandList *GetCommandList()
    {
        if (commandList->IsSuitableSubmitted())
        {
            Execute();
        }

        if (!allocator || !commandList->IsState(CommandList::State::Reset))
        {
            __Begin();
        }

        return commandList;
    }

    template <class T>
    requires std::is_same_v<Fence, T> || std::is_same_v<Queue, T>
    T *GetAddress() const
    {
        if constexpr (std::is_same_v<Fence, T>)
        {
            return fence;
        }
        if constexpr (std::is_same_v<Queue, T>)
        {
            return queue;
        }
    }

    uint64_t GetFenceValue() const
    {
        return fenceValue;
    }

    uint64_t IncreaseFence()
    {
        return ++fenceValue;
    }

private:
    void __Begin();

    void __Release();

    void __InjectSignal();

protected:
    URef<Queue> queue;

	URef<CommandAllocatorPool> allocatorPool;

	URef<CommandList> commandList;

    URef<Fence> fence;

    CommandAllocator *allocator;

    uint64_t fenceValue;

    std::list<ID3D12Resource *> resources;

    std::queue<std::pair<uint64_t, std::list<ID3D12Resource *>>> resourceCache;
};

}
}
