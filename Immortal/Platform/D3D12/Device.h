#pragma once

#include "Framework/Utils.h"
#include "Common.h"

#include "Queue.h"
#include "Descriptor.h"

namespace Immortal
{
namespace D3D12
{

class Device
{
public: 
    using Super = ID3D12Device;

public:
    Device(ComPtr<IDXGIFactory4> factory);

    ~Device();

    DXGI_ADAPTER_DESC GetAdapterDesc();

public:
    ID3D12Device *Handle()
    {
        return handle;
    }

    operator const ID3D12Device*() const
    {
        return handle;
    }

    operator ID3D12Device*()
    {
        return handle;
    }

    template <class T>
    T *GetAddress() const
    {
        if (IsPrimitiveOf<IDXGIFactory4, T>())
        {
            return dxgiFactory;
        }
    }

#define DEFINE_CRETE_FUNC(U, T, O) \
    HRESULT Create(const D3D12_##U##_DESC *pDesc, ID3D12##T **ppObject) \
    { \
        return handle->Create##O(pDesc, IID_PPV_ARGS(ppObject)); \
    } \
    HRESULT Create##O(const D3D12_##U##_DESC *pDesc, ID3D12##T **ppObject) \
    { \
        return handle->Create##O(pDesc, IID_PPV_ARGS(ppObject)); \
    } \

#define DEFINE_CREATE_OBJECT(U, T)    DEFINE_CRETE_FUNC(U, T, T)
    DEFINE_CREATE_OBJECT(COMMAND_QUEUE,   CommandQueue  )
    DEFINE_CREATE_OBJECT(DESCRIPTOR_HEAP, DescriptorHeap)

#define DEFINE_CREATE_PREFIX(U, T, P) DEFINE_CRETE_FUNC(U, T, P##T)
    DEFINE_CREATE_PREFIX(GRAPHICS_PIPELINE_STATE, PipelineState, Graphics)
    DEFINE_CREATE_PREFIX(COMPUTE_PIPELINE_STATE,  PipelineState, Compute )

    uint32_t GetDescriptorIncrement(const D3D12_DESCRIPTOR_HEAP_TYPE &type) const
    {
        return handle->GetDescriptorHandleIncrementSize(type);
    }

#define DEFINE_CREATE_VIEW(T, F) \
    void CreateView(ID3D12Resource *p##F, D3D12_##T##_VIEW_DESC *pDesc, CPUDescriptor &descriptor) const \
    { \
        handle->Create##F##View(p##F, pDesc, descriptor); \
    }

    DEFINE_CREATE_VIEW(RENDER_TARGET,   RenderTarget  )
    DEFINE_CREATE_VIEW(SHADER_RESOURCE, ShaderResource)
    DEFINE_CREATE_VIEW(DEPTH_STENCIL,   DepthStencil  )

    void CreateView(ID3D12Resource *pResource,
        ID3D12Resource *pCounterResouce,
        const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc,
        CPUDescriptor descriptor)
    {
        handle->CreateUnorderedAccessView(pResource, pCounterResouce, pDesc, descriptor);
    }

    void CreateView(const D3D12_CONSTANT_BUFFER_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        handle->CreateConstantBufferView(pDesc, descriptor);
    }

    void Create(UINT nodeMask, ID3DBlob *pSignature, ID3D12RootSignature **ppRootSignature)
    {
        Check(handle->CreateRootSignature(
            nodeMask,
            pSignature->GetBufferPointer(),
            pSignature->GetBufferSize(),
            IID_PPV_ARGS(ppRootSignature)
            ));
    }

    void CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator **ppAllocator) const
    {
        Check(handle->CreateCommandAllocator(
            type,
            IID_PPV_ARGS(ppAllocator)
            ));
    }

    void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pipeleState, ID3D12GraphicsCommandList **ppCommandList, UINT nodeMask = 0) const
    {
        Check(handle->CreateCommandList(
            nodeMask,
            type,
            pAllocator,
            pipeleState,
            IID_PPV_ARGS(ppCommandList)
            ));
    }

    void CreateFence(ID3D12Fence **pfence, UINT64 initialValue = 0, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE) const
    {
        Check(handle->CreateFence(
            initialValue,
            flag,
            IID_PPV_ARGS(pfence)
            ));
    }

    void CreateCommittedResource(
        const D3D12_HEAP_PROPERTIES *pHeapProperties,
              D3D12_HEAP_FLAGS       heapFlags,
        const D3D12_RESOURCE_DESC   *pDesc,
              D3D12_RESOURCE_STATES  initialResourceState,
        const D3D12_CLEAR_VALUE     *pOptimizedClearValue,
              ID3D12Resource       **pResource) const
    {
        Check(handle->CreateCommittedResource(
            pHeapProperties,
            heapFlags,
            pDesc,
            initialResourceState,
            pOptimizedClearValue,
            IID_PPV_ARGS(pResource)
            ));
    }
    
    void CopyDescriptors(
        UINT numDestDescriptorRanges,
        const D3D12_CPU_DESCRIPTOR_HANDLE *pDestDescriptorRangeStarts,
        const UINT *pDestDescriptorRangeSizes,
        UINT numSrcDescriptorRanges,
        const D3D12_CPU_DESCRIPTOR_HANDLE *pSrcDescriptorRangeStarts,
        const UINT *pSrcDescriptorRangeSizes,
        D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapsType)
    {
        handle->CopyDescriptors(
            numDestDescriptorRanges,
            pDestDescriptorRangeStarts,
            pDestDescriptorRangeSizes,
            numSrcDescriptorRanges,
            pSrcDescriptorRangeStarts,
            pSrcDescriptorRangeSizes,
            descriptorHeapsType
            );
    }

    void CopyDescriptors(
        UINT                        numDescriptors,
        D3D12_CPU_DESCRIPTOR_HANDLE destDescriptorRangeStart,
        D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptorRangeStart,
        D3D12_DESCRIPTOR_HEAP_TYPE  descriptorHeapsType)
    {
        handle->CopyDescriptorsSimple(
            numDescriptors,
            destDescriptorRangeStart,
            srcDescriptorRangeStart,
            descriptorHeapsType
            );
    }

    void Set(const std::wstring &name)
    {
        handle->SetName(name.c_str());
    }

    void Set(const std::string &name)
    {
        auto wName = Utils::s2ws(name);
        Set(wName);
    }

private:
    ID3D12Device *handle{ nullptr };

    IDXGIFactory4 *dxgiFactory{ nullptr };

    static inline bool UseWarpDevice{ false };
};

}
}
