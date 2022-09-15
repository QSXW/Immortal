#pragma once

#include "Core.h"

#include "Buffer.h"
#include "Descriptor.h"
#include "Shader.h"
#include "RenderTarget.h"
#include "Interface/IObject.h"

namespace Immortal
{

class GraphicsPipeline;
class ComputePipeline;
class IMMORTAL_API Pipeline : public IObject
{
public:
    enum State : uint32_t
    {
        Depth = BIT(0),
        Blend = BIT(1),
    };

    enum class DrawType
    {
        None = 0,
        Static,
        Stream,
        Dynamic
    };

    enum class PrimitiveType
    {
        Point,
        Line,
        Triangles
    };

    using Type = Shader::Type;

    using Graphics = GraphicsPipeline;
    using Compute = ComputePipeline;

public:
    virtual ~Pipeline()
    {

    }

    virtual void Bind(Buffer *buffer, uint32_t slot = 0)
    {

    }

    virtual void Bind(Texture *texture, uint32_t slot = 0)
    {

    }

    virtual void Bind(const std::string &name, const Buffer *uniform)
    {

    }

    virtual void Bind(const DescriptorBuffer *descriptorBuffer, uint32_t slot = 0)
    {

    }

    virtual Anonymous AllocateDescriptorSet(uint64_t uuid)
    {
        return nullptr;
    }

    virtual void FreeDescriptorSet(uint64_t uuid)
    {

    }

public:
    void Enable(uint32_t request)
    {
        flags |= request;
    }

    void Disable(uint32_t request)
    {
        flags &= ~request;
    }

protected:
    uint32_t flags = State::Depth;
};

class IMMORTAL_API GraphicsPipeline : public virtual Pipeline
{
public:
    GraphicsPipeline()
    {

    }

    GraphicsPipeline(Ref<Shader> shader) :
        desc{ shader }
    {

    }

    virtual ~GraphicsPipeline() { }

    virtual void Map() { }

    virtual void Unmap() { }

    virtual void Set(Ref<Buffer> buffer)
    {
        if (buffer->GetType() == Buffer::Type::Vertex)
        {
            desc.vertexBuffers.resize(1);
            desc.vertexBuffers[0] = buffer;
        }
        if (buffer->GetType() == Buffer::Type::Index)
        {
            desc.indexBuffer = buffer;
            ElementCount = buffer->Count();
        }
    }

    virtual void Set(const InputElementDescription &description)
    {
        desc.layout = description;
    }

    virtual void Create(const RenderTarget *renderTarget)
    {

    }

    virtual void Reconstruct(const RenderTarget *renderTarget)
    {

    }

    virtual void CopyState(GraphicsPipeline &other)
    {

    }

    template <class T>
    void Update(size_t size, const T *data, int slot = 0)
    {
        desc.vertexBuffers[slot]->Update(sizeof(T) * size, rcast<const void*>(data));
    }

    template <class T>
    Ref<T> VertexBuffer(size_t index = 0)
    {
        return desc.vertexBuffers[index];
    }

    template <class T>
    Ref<T> IndexBuffer()
    {
        return desc.indexBuffer;
    }

protected:
    struct Description
    {
        Ref<Shader> shader;

        std::vector<Ref<Buffer>> vertexBuffers;

        Ref<Buffer> indexBuffer;

        InputElementDescription layout{};

        DrawType type{ DrawType::Static };

        PrimitiveType primitiveType = PrimitiveType::Triangles;
    } desc;

public:
    uint32_t ElementCount = 0;
};

class IMMORTAL_API ComputePipeline : public virtual Pipeline
{
public:
    virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ = 0) = 0;

    virtual void PushConstant(uint32_t size, const void *data, uint32_t offset = 0) = 0;
};

using SuperPipeline         = Pipeline;
using SuperComputePipeline  = ComputePipeline;
using SuperGraphicsPipeline = GraphicsPipeline;

namespace Interface
{
    using Pipeline         = SuperPipeline;
    using ComputePipeline  = SuperComputePipeline;
    using GraphicsPipeline = SuperGraphicsPipeline; 
}

SL_DEFINE_BITWISE_OPERATION(Pipeline::State, uint32_t)

}
