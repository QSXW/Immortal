#pragma once

#include "Common.h"
#include "Algorithm/LightArray.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/Pipeline.h"
#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"
#include "Handle.h"
#include "Config.h"
#include <queue>

namespace Immortal
{
namespace D3D12
{

class PipelineStateSubobjectType
{
public:
	PipelineStateSubobjectType(const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE &type) :
	    Type{type}
	{

	}

	D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type;
};

template <class T>
class PipelineStateValue
{
public:
	T Value;

    PipelineStateValue() :
	    Value{}
	{
    
    }

    PipelineStateValue(const T &v) :
	    Value{v}
	{

	}

	operator T&()
	{
		return Value;
	}

	PipelineStateValue &operator=(const T &v)
	{
		Value = v;
		return *this;
	};
};

#pragma warning(push)
#pragma warning(disable : 4324)
template <class T, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE _Type>
class alignas(void *) PipelineStateStreamSubobject : public PipelineStateSubobjectType, public T
{
public:
	PipelineStateStreamSubobject() noexcept :
	    PipelineStateSubobjectType(_Type),
	    T{}
	{

    }

	PipelineStateStreamSubobject(T const &i) noexcept :
	    PipelineStateSubobjectType(_Type), T(i)
	{

    }

    operator T &() noexcept
	{
		return Value();
	}

    PipelineStateStreamSubobject &operator = (const T &v) noexcept
    {
		auto &value = Value();
		value = v;
		return *this;
    }

protected:
    T &Value() noexcept
	{
        if constexpr (sizeof(T) <= 4)
        {
			return *(T *) (((uint8_t *) this) + sizeof(PipelineStateSubobjectType));
        }
        else
		{
			return *(T *) (((uint8_t *) this) + sizeof(void *));
        }
	}
};
#pragma warning(pop)

using PipelineStateStreamFlags                     = PipelineStateStreamSubobject<PipelineStateValue<D3D12_PIPELINE_STATE_FLAGS>,    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS>;
using PipelineStateStreamNodeMask                  = PipelineStateStreamSubobject<PipelineStateValue<UINT>,                          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK>;
using PipelineStateStreamRootSignature             = PipelineStateStreamSubobject<PipelineStateValue<ID3D12RootSignature*>,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE>;
using PipelineStateStreamInputLayout               = PipelineStateStreamSubobject<D3D12_INPUT_LAYOUT_DESC,                           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT>;
using PipelineStateStreamIndexBufferStripCutValue  = PipelineStateStreamSubobject<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE,                D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE>;
using PipelineStateStreamPrimitiveTopologyType     = PipelineStateStreamSubobject<PipelineStateValue<D3D12_PRIMITIVE_TOPOLOGY_TYPE>, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY>;
using PipelineStateStreamVS                        = PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>;
using PipelineStateStreamGS                        = PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS>;
using PipelineStateStreamStreamOutput              = PipelineStateStreamSubobject<D3D12_STREAM_OUTPUT_DESC,                          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT>;
using PipelineStateStreamHS                        = PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS>;
using PipelineStateStreamDS                        = PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS>;
using PipelineStateStreamPS                        = PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>;
using PipelineStateStreamAS                        = PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS>;
using PipelineStateStreamMS                        = PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>;
using PipelineStateStreamCS                        = PipelineStateStreamSubobject<D3D12_SHADER_BYTECODE,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS>;
using PipelineStateStreamBlendDesc                 = PipelineStateStreamSubobject<BlendDescription,                                  D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND>;
using PipelineStateStreamDepthStencil              = PipelineStateStreamSubobject<DepthStencilDescription,                           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL>;
using PipelineStateStreamDepthStencilFormat        = PipelineStateStreamSubobject<PipelineStateValue<DXGI_FORMAT>,                   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT>;
using PipelineStateStreamRasterizer                = PipelineStateStreamSubobject<RasterizerDescription,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER>;
using PipelineStateStreamRenderTargetFormats       = PipelineStateStreamSubobject<D3D12_RT_FORMAT_ARRAY,                             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>;
using PipelineStateStreamSampleDesc                = PipelineStateStreamSubobject<DXGI_SAMPLE_DESC,                                  D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC>;
using PipelineStateStreamSampleMask                = PipelineStateStreamSubobject<PipelineStateValue<UINT>,                          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK>;
using PipelineStateStreamCachedPipelineState       = PipelineStateStreamSubobject<D3D12_CACHED_PIPELINE_STATE,                       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO>;
using PipelineStateStreamViewInstancing            = PipelineStateStreamSubobject<ViewInstancingDescription,                         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING>;

struct PipelineMeshStateStream
{
	PipelineStateStreamFlags                 Flags;
	PipelineStateStreamNodeMask              NodeMask;
	PipelineStateStreamRootSignature         pRootSignature;
	PipelineStateStreamPS                    PS;
	PipelineStateStreamAS                    AS;
	PipelineStateStreamMS                    MS;
	PipelineStateStreamPrimitiveTopologyType PrimitiveTopologyType;
	PipelineStateStreamBlendDesc             BlendState;
	PipelineStateStreamDepthStencil          DepthStencilState;
	PipelineStateStreamDepthStencilFormat    DSVFormat;
	PipelineStateStreamRasterizer            RasterizerState;
	PipelineStateStreamRenderTargetFormats   RTVFormats;
	PipelineStateStreamSampleDesc            SampleDesc;
	PipelineStateStreamSampleMask            SampleMask;
	PipelineStateStreamCachedPipelineState   CachedPSO;
	PipelineStateStreamViewInstancing        ViewInstancingDesc;
};

class Device;
class Shader;
class Pipeline : public virtual SuperPipeline, public NonDispatchableHandle
{
public:
    using Super = SuperPipeline;

    enum class Type
    {
        Graphics,
        Compute
    };

    struct DescriptorTable
    {
		uint32_t RootParameterIndex;
        uint32_t DescriptorCount;
        uint32_t Offset;
		D3D12_DESCRIPTOR_HEAP_TYPE HeapType;
    };

    using Primitive = ID3D12PipelineState;
    D3D12_OPERATOR_HANDLE()

public:
	Pipeline(Device *device, Type type = Type::Graphics);

    virtual ~Pipeline();

    void ConstructRootParameter(Shader *shader, std::vector<RootParameter> &rootParameters, std::vector<D3D12_DESCRIPTOR_RANGE1> &ranges, std::vector<D3D12_DESCRIPTOR_RANGE1> &samplerRanges, std::vector<D3D12_STATIC_SAMPLER_DESC> *pSamplerDesc = {});

    void ConstructRootSignature(Shader **ppShader, size_t shaderCount);

    void AddDescriptorTable(std::vector<RootParameter> &rootParameters, std::vector<DescriptorTable> &descriptorTables, std::vector<D3D12_DESCRIPTOR_RANGE1> &ranges, D3D12_DESCRIPTOR_HEAP_TYPE heapType);

public:
    const RootSignature &GetRootSignature() const
    {
		return *rootSignature;
    }

    Type GetType() const
    {
		return type;
    }

    const std::vector<DescriptorTable> &GetDescriptorTables() const
    {
        return descriptorTables;
    }

    bool HasRootConstant() const
    {
		return hasRootConstant;
    }

    uint32_t GetDescriptorCount(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
		return descriptorCount[type];
    }

    const uint32_t *GetDescriptorIndexMap(D3D12_DESCRIPTOR_RANGE_TYPE type) const
    {
		return descriptorIndexMap[type].data();
    }

    const D3D12_DESCRIPTOR_RANGE_TYPE *GetDescriptorRangeType() const
    {
		return descriptorRangeType.data();
    }

    uint32_t GetPushConstantRootParameterIndex(D3D12_SHADER_VISIBILITY visibility)
    {
		return shaderIndexes[visibility].pushConstant;
    }

protected:
	Type type;

    std::vector<uint32_t> descriptorIndexMap[D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER + 1];

    uint32_t descriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

    std::vector<D3D12_DESCRIPTOR_RANGE_TYPE> descriptorRangeType;

    std::vector<DescriptorTable> descriptorTables;

    URef<RootSignature> rootSignature;

    ShaderVisibilityIndex shaderIndexes[D3D12_SHADER_VISIBILITY_MESH + 1] = {};

    bool hasRootConstant = false;
};

class GraphicsPipeline : public Pipeline, public SuperGraphicsPipeline
{
public:
    using Super = SuperGraphicsPipeline;

public:
    GraphicsPipeline(Device *device, Ref<Shader::Super> shader);

    GraphicsPipeline(Device *device, Ref<Shader> shader);

    GraphicsPipeline(Device *device);

    virtual ~GraphicsPipeline() override;

	virtual void Construct(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription) override;

    void ConstructGraphicsPipeline(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription);

    void ConstructMeshPipeline(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription);

protected:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ConstructDescription();

	void SetInputElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescriptions, const InputElementDescription &description);

    UINT ConstructRenderTargetFormats(const std::vector<Format> &outputDescription, DXGI_FORMAT *rtvFormats, DXGI_FORMAT &dsvFormat, D3D12_DEPTH_STENCIL_DESC &depthDesc, D3D12_BLEND_DESC &blendState);

public:
    uint32_t GetVertexEntryStride() const
    {
		return vertexEntryStride;
    }

    D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const
    {
		return primitiveTopology;
    }

protected:
    uint32_t vertexEntryStride;

    D3D_PRIMITIVE_TOPOLOGY primitiveTopology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
};

class WorkGraphContext
{
public:
	WorkGraphContext(Device *device, ComPtr<ID3D12StateObject> stateObject, LPCWSTR pWorkGraphName);

    ~WorkGraphContext();

	Ref<Buffer> backingMemory;
	D3D12_PROGRAM_IDENTIFIER programIdentifier = {};
	D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS memoryRequirements = {};
};

class ComputePipeline : public Pipeline, public SuperComputePipeline
{
public:
    using Super = SuperComputePipeline;

public:
    ComputePipeline(Device *device, SuperShader *shader);

    ComputePipeline(Device *device, Shader *shader);

    virtual ~ComputePipeline() override;

    void CreateWorkGraph(Shader *shader);

    D3D12_SET_PROGRAM_DESC GetSetProgramDesc() const;

protected:
#if HAVE_AGILITY_SDK
	URef<WorkGraphContext> workgraph;

    ComPtr<ID3D12StateObject> stateObject;
#endif
};

}
}
