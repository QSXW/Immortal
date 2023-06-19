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

#include <queue>

namespace Immortal
{
namespace D3D12
{

class RenderContext;
class Pipeline : public virtual SuperPipeline
{
public:
    using Super = SuperPipeline;

    struct DescriptorTable
    {
		uint32_t RootParameterIndex;
        uint32_t DescriptorCount;
        uint32_t Offset;
    };

    using Primitive = ID3D12PipelineState;
    D3D12_OPERATOR_HANDLE()

public:
    Pipeline(RenderContext *context);

    virtual ~Pipeline();

    virtual void Bind(const DescriptorBuffer *descriptors, uint32_t binding = 0) override;

    virtual void Bind(SuperBuffer *buffer, uint32_t binding = 0) override;

    virtual void Bind(SuperTexture *texture, uint32_t binding = 0) override;

    virtual Anonymous AllocateDescriptorSet(uint64_t uuid) override;

    virtual void FreeDescriptorSet(uint64_t uuid) override;

    void Bind(Buffer *buffer, uint32_t binding = 0);

    void Bind(const CPUDescriptor *descriptors, uint32_t binding = 0);

    void InitRootSignature(const Shader *shader);

public:
    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<DescriptorHeap, T>())
        {
            return descriptorHeap.active;
        }
    }

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<RootSignature, T>())
        {
            return *rootSignature;
        }
    }

    const std::vector<DescriptorTable> &GetDescriptorTables() const
    {
        return descriptorTables;
    }

    bool HasRootConstant() const
    {
		return hasRootConstant;
    }

protected:
    RenderContext *context{};

    struct {
        DescriptorHeap *active = nullptr;
        std::unordered_map <uint64_t, URef<DescriptorHeap>> packs;
        std::queue<DescriptorHeap*> freePacks;
    } descriptorHeap;

    uint32_t textureIndexInDescriptorTable = 0;
    std::vector<DescriptorTable> descriptorTables;
    
    URef<RootSignature> rootSignature;

    bool hasRootConstant = false;
};

class GraphicsPipeline : public Pipeline, public SuperGraphicsPipeline
{
public:
    using Super = SuperGraphicsPipeline;

    struct State
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescription;
    };

public:
    GraphicsPipeline(RenderContext *context, Ref<Shader::Super> shader);

    GraphicsPipeline(RenderContext *context, Ref<Shader> shader);

    virtual void Create(const SuperRenderTarget *renderTarget) override;

    virtual void Reconstruct(const SuperRenderTarget *renderTarget) override;

    virtual void Set(Ref<Buffer::Super> buffer) override;

    virtual void Set(const InputElementDescription &description) override;

	void Create(ID3D12Resource *resource);

protected:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC __GetDescription();

public:
    template <class T, Buffer::Type type = Buffer::Type::Index>
    T Get()
    {
        if constexpr (IsPrimitiveOf<Buffer::VertexView, T>())
        {
            return bufferViews.vertex;
        }
        if constexpr (IsPrimitiveOf<Buffer::IndexView, T>())
        {
            return bufferViews.index;
        }
        if constexpr (IsPrimitiveOf<D3D_PRIMITIVE_TOPOLOGY, T>())
        {
            return primitiveTopology;
        }
        if constexpr (IsPrimitiveOf<RootSignature, T>())
        {
            return *rootSignature;
        }
    }

    template <Buffer::Type type>
    Buffer *GetBuffer()
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            return dynamic_cast<Buffer*>(desc.vertexBuffers[0].Get());
        }
        if constexpr (type == Buffer::Type::Index)
        {
            return dynamic_cast<Buffer*>(desc.indexBuffer.Get());
        }
    }

private:
    struct {
        Buffer::VertexView vertex;
        Buffer::IndexView index;
    } bufferViews;

    D3D_PRIMITIVE_TOPOLOGY primitiveTopology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

    std::unique_ptr<State> state;
};

class ComputePipeline : public Pipeline, public SuperComputePipeline
{
public:
    using Super = SuperComputePipeline;

public:
    ComputePipeline(RenderContext *context, Shader::Super *shader);

    void Dispatch(CommandList *cmdlist, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ = 0);

    void PushConstant(CommandList *cmdlist, uint32_t size, const void *data, uint32_t offset = 0);

protected:
    LightArray<uint8_t, 128> pushConstants;
};

}
}
