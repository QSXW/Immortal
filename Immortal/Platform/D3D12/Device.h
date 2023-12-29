#pragma once

#include "Framework/Utils.h"
#include "Common.h"

#include "Queue.h"
#include "Descriptor.h"
#include "Render/Sampler.h"
#include "Render/DescriptorSet.h"
#include "PhysicalDevice.h"

#include <unordered_map>
#include <shared_mutex>

namespace Immortal
{

namespace D3D12
{

struct AdapterProperty
{
	ComPtr<IDXGIAdapter1> pAdapter;
	DXGI_ADAPTER_DESC desc;
};

class DescriptorPool;
class DescriptorHeap;
class Sampler;
class Pipeline;
class IMMORTAL_API Device : public SuperDevice
{
public: 
    using Primitive = ID3D12Device;
	D3D12_OPERATOR_HANDLE()

public:
	Device(PhysicalDevice *physicalDevice);

    virtual ~Device() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual BackendAPI GetBackendAPI() override;

    virtual SuperQueue *CreateQueue(QueueType type, QueuePriority priority) override;

    virtual SuperCommandBuffer *CreateCommandBuffer(QueueType type) override;

    virtual SuperSwapchain *CreateSwapchain(SuperQueue *queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode) override;
        
    virtual SuperSampler *CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod) override;

    virtual SuperShader *CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint) override;

    virtual SuperGraphicsPipeline *CreateGraphicsPipeline() override;

    virtual SuperComputePipeline *CreateComputePipeline(SuperShader *shader) override;

    virtual SuperTexture *CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type) override;

    virtual SuperBuffer *CreateBuffer(size_t size, BufferType type) override;

    virtual SuperDescriptorSet *CreateDescriptorSet(SuperPipeline *pipeline) override;

    virtual SuperGPUEvent *CreateGPUEvent(const std::string &name) override;

    virtual SuperRenderTarget *CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat = {}) override;

public:
	IDXGIAdapter1 *GetAdapter() const;

	IDXGIFactory4 *GetDXGIFactory() const;

    void CreateSampler(const D3D12_SAMPLER_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE *pDestDescriptor);

    Pipeline *GetPipeline(const std::string &name);

    Sampler *GetSampler(Filter filter);

    Descriptor AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorCount = 1);

    void AllocateShaderVisibleDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, DescriptorHeap **ppHeap, ShaderVisibleDescriptor *pBaseDescriptor, uint32_t descriptorCount);

    void Destory(ID3D12Resource *pResource);

public:
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

	void CreateRenderTargetView(ID3D12Resource *pRenderTarget, const D3D12_RENDER_TARGET_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE descriptor) const
	{
		handle->CreateRenderTargetView(pRenderTarget, pDesc, descriptor);
	}

	void CreateShaderResourceView(ID3D12Resource *pShaderResource, const D3D12_SHADER_RESOURCE_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE descriptor) const
	{
		handle->CreateShaderResourceView(pShaderResource, pDesc, descriptor);
	}

	void CreateDepthStencilView(ID3D12Resource *pDepthStencil, const D3D12_DEPTH_STENCIL_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE descriptor) const
	{
		handle->CreateDepthStencilView(pDepthStencil, pDesc, descriptor);
	}

    void CreateUnorderedAccessView(ID3D12Resource *pResource, ID3D12Resource *pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
    {
		handle->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, destDescriptor);
    }

    void CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        handle->CreateConstantBufferView(pDesc, descriptor);
    }

    template <class T>
    HRESULT QueryInterface(T **ppObject)
    {
		return handle->QueryInterface(IID_PPV_ARGS(ppObject));
    }

    HRESULT Create(ID3DBlob *pSignature, ID3D12RootSignature **ppRootSignature, UINT nodeMask = 0)
    {
        return handle->CreateRootSignature(
            nodeMask,
            pSignature->GetBufferPointer(),
            pSignature->GetBufferSize(),
            IID_PPV_ARGS(ppRootSignature)
            );
    }

    HRESULT CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator **ppAllocator) const
    {
        return handle->CreateCommandAllocator(
            type,
            IID_PPV_ARGS(ppAllocator)
            );
    }

    template <class T>
	requires std::is_base_of_v<ID3D12CommandList, T>
    HRESULT CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pipeleState, T **ppCommandList, UINT nodeMask = 0) const
    {
        return handle->CreateCommandList(
            nodeMask,
            type,
            pAllocator,
            pipeleState,
            IID_PPV_ARGS(ppCommandList)
            );
    }

    HRESULT CreateFence(ID3D12Fence **pfence, UINT64 initialValue = 0, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE) const
    {
        return handle->CreateFence(
            initialValue,
            flag,
            IID_PPV_ARGS(pfence)
            );
    }

    HRESULT CreateCommittedResource(
        const D3D12_HEAP_PROPERTIES *pHeapProperties,
              D3D12_HEAP_FLAGS       heapFlags,
        const D3D12_RESOURCE_DESC   *pDesc,
              D3D12_RESOURCE_STATES  initialResourceState,
        const D3D12_CLEAR_VALUE     *pOptimizedClearValue,
              ID3D12Resource       **pResource) const
    {
        return handle->CreateCommittedResource(
            pHeapProperties,
            heapFlags,
            pDesc,
            initialResourceState,
            pOptimizedClearValue,
            IID_PPV_ARGS(pResource)
            );
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

    void GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo)
    {
        ComPtr<ID3D12Device5> pDevice;
        handle.As(&pDevice);
        pDevice->GetRaytracingAccelerationStructurePrebuildInfo(pDesc, pInfo);
    }

    bool IsRayTracingSupported() const
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 features{};
        Check(handle->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features, sizeof(features)));
        return features.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }

protected:
	PhysicalDevice *physicalDevice;

	URef<DescriptorPool> descriptorPools[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    URef<DescriptorPool> shaderVisibleDescriptorPools[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    struct
    {
		URef<Sampler> nearest;
		URef<Sampler> linear;
    } samplers;

    std::shared_mutex pipelineMutex;
    std::unordered_map<std::string, URef<Pipeline>> pipelines;

    std::mutex resourceMutex;
	std::vector<ID3D12Resource *> resources;
};

}
}
