#pragma once

#include "Common.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Pipeline : public virtual SuperPipeline
{
public:
    using Super = SuperPipeline;

    struct DescriptorTable
    {
        uint32_t DescriptorCount;
        uint32_t Offset;
    };

public:
    virtual ~Pipeline();

    virtual void Bind(const std::string &name, const Buffer::Super *uniform) override;

    virtual void Bind(const Descriptor::Super *descriptors, uint32_t binding = 0) override;

    virtual void Bind(Buffer::Super *buffer, uint32_t slot = 0) override;

    virtual void Bind(Texture::Super *texture, uint32_t slot = 0) override;

    virtual Anonymous AllocateDescriptorSet(uint64_t uuid) override;

    virtual void FreeDescriptorSet(uint64_t uuid) override;

    void Bind(Buffer *buffer, uint32_t slot = 0);

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
            return rootSignature;
        }
    }

    operator ID3D12PipelineState*()
    {
        return pipelineState;
    }

    const std::vector<DescriptorTable> &GetDescriptorTables() const
    {
        return descriptorTables;
    }

protected:
    Device *device{ nullptr };

    ID3D12PipelineState *pipelineState{ nullptr };

    struct {
        DescriptorHeap *active = nullptr;
        std::unordered_map <uint64_t, DescriptorHeap*> packs;
        std::queue<DescriptorHeap*> freePacks;
    } descriptorHeap;

    uint32_t textureIndexInDescriptorTable = 0;
    std::vector<DescriptorTable> descriptorTables;
    
    RootSignature rootSignature;
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
    GraphicsPipeline(Device *device, Ref<Shader::Super> shader);

    virtual void Create(const SuperRenderTarget *renderTarget) override;

    virtual void Reconstruct(const SuperRenderTarget *renderTarget) override;

    virtual void Set(Ref<Buffer::Super> buffer) override;

    virtual void Set(const InputElementDescription &description) override;

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
            return rootSignature;
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
    ComputePipeline(Device *device, Shader::Super *shader);

    virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ = 0) override;

private:
};

}
}
