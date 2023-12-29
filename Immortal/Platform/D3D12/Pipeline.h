#pragma once

#include "Common.h"
#include "Algorithm/LightArray.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"
#include "Handle.h"

#include <queue>

namespace Immortal
{
namespace D3D12
{

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

    void ConstructRootParameter(Shader *shader, std::vector<RootParameter> *pRootParameters, std::vector<D3D12_STATIC_SAMPLER_DESC> *pSamplerDesc = {});

    void ConstructRootSignature(Shader **ppShader, size_t shaderCount);

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

    const uint32_t *GetDescriptorIndexMap(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
		return descriptorIndexMap[type].data();
    }

protected:
	Type type;

    std::vector<uint32_t> descriptorIndexMap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    uint32_t descriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

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

protected:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ConstructDescription();

	void SetInputElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescriptions, const InputElementDescription &description);

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

class ComputePipeline : public Pipeline, public SuperComputePipeline
{
public:
    using Super = SuperComputePipeline;

public:
    ComputePipeline(Device *device, SuperShader *shader);

    ComputePipeline(Device *device, Shader *shader);

    virtual ~ComputePipeline() override;
};

}
}
