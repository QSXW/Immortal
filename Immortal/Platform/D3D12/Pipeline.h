#pragma once

#include "Common.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Texture.h"
#include "Buffer.h"
#include "Shader.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Pipeline : public SuperGraphicsPipeline
{
public:
    using Super = SuperGraphicsPipeline;

    struct DescriptorTable
    {
        uint32_t DescriptorCount;
        uint32_t Offset;
    };

    struct State
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescription;
    };

    static D3D12_PRIMITIVE_TOPOLOGY_TYPE SuperToBase(const PrimitiveType type)
    {
        switch (type)
        {
        case PrimitiveType::Line:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

        case PrimitiveType::Point:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

        case PrimitiveType::Triangles:
        default:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        }
    }

public:
    Pipeline(Device *device, std::shared_ptr<Shader::Super> shader);

    virtual ~Pipeline();

    virtual void Create(const std::shared_ptr<SuperRenderTarget> &renderTarget, Option option = Option{}) override;

    virtual void Reconstruct(const std::shared_ptr<SuperRenderTarget> &renderTarget, Option option = Option{}) override;

    virtual void Set(std::shared_ptr<Buffer::Super> &buffer) override;

    virtual void Set(const InputElementDescription &description) override;

    virtual void Bind(const std::string &name, const Buffer::Super *uniform) override;

    virtual void Bind(const Descriptor::Super *descriptors, uint32_t binding = 0) override;

    virtual void Bind(Texture *texture, uint32_t slot = 0);

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

    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<DescriptorAllocator, T>())
        {
            return &descriptorAllocator;
        }
    }

    template <Buffer::Type type>
    Buffer *GetBuffer()
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            return std::dynamic_pointer_cast<Buffer>(desc.vertexBuffers[0]).get();
        }
        if constexpr (type == Buffer::Type::Index)
        {
            return std::dynamic_pointer_cast<Buffer>(desc.indexBuffer).get();
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

private:
    Device *device{ nullptr };

    ID3D12PipelineState *pipelineState{ nullptr };

    DescriptorAllocator descriptorAllocator;

    std::vector<DescriptorTable> descriptorTables;

    RootSignature rootSignature;

    struct {
        Buffer::VertexView vertex;
        Buffer::IndexView index;
    } bufferViews;

    D3D_PRIMITIVE_TOPOLOGY primitiveTopology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

    std::unique_ptr<State> state;
};

}
}
