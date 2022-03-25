#pragma once

#include "Core.h"

#include "Buffer.h"
#include "Descriptor.h"
#include "Shader.h"
#include "RenderTarget.h"

namespace Immortal
{

class GraphicsPipeline;
class ComputePipeline;
class IMMORTAL_API Pipeline
{
public:
    enum class Feature
    {
        None  = 0,
        DepthDisabled = BIT(1),
        BlendEnabled  = BIT(2)
    };

    struct Option
    {
        Feature flags = Feature::None;
    };

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

    virtual void Bind(const Descriptor *descriptors, uint32_t slot = 0)
    {

    }

    virtual Anonymous AllocateDescriptorSet(uint64_t uuid)
    {
        return nullptr;
    }

    virtual void FreeDescriptorSet(uint64_t uuid)
    {

    }

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
    using Compute  = ComputePipeline;
};

class IMMORTAL_API GraphicsPipeline : public virtual Pipeline
{
public:
    GraphicsPipeline()
    {

    }

    GraphicsPipeline(std::shared_ptr<Shader> &shader) :
        desc{ shader }
    {

    }

    virtual ~GraphicsPipeline() { }

    virtual void Map() { }

    virtual void Unmap() { }

    virtual void Set(std::shared_ptr<Buffer> buffer)
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

    virtual void Create(const std::shared_ptr<RenderTarget> &renderTarget, Option option = Option{})
    {

    }

    virtual void Reconstruct(const std::shared_ptr<RenderTarget> &renderTarget, Option option = Option{})
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
    std::shared_ptr<T> VertexBuffer(size_t index = 0)
    {
        return std::dynamic_pointer_cast<T>(desc.vertexBuffers(index));
    }

    template <class T>
    std::shared_ptr<T> IndexBuffer()
    {
        return std::dynamic_pointer_cast<T>(desc.indexBuffer);
    }

protected:
    struct Description
    {
        std::shared_ptr<Shader> shader{ nullptr };

        std::vector<std::shared_ptr<Buffer>> vertexBuffers;

        std::shared_ptr<Buffer> indexBuffer;

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

    virtual void ResetResource()
    {

    }

    template <class T>
    void Sumbmit(T &&func)
    {
        ResetResource();
        func();
    }

    virtual void PushConstant(uint32_t size, const void *data, uint32_t offset = 0)
    {

    }
};

using SuperPipeline         = Pipeline;
using SuperComputePipeline  = ComputePipeline;
using SuperGraphicsPipeline = GraphicsPipeline;

SL_DEFINE_BITWISE_OPERATION(Pipeline::Feature, uint32_t)

}
