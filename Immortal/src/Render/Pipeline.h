#pragma once

#include "ImmortalCore.h"

#include "Buffer.h"
#include "Shader.h"
#include "RenderTarget.h"

namespace Immortal
{

class IMMORTAL_API Pipeline
{
public:
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

public:
    Pipeline() { }

    Pipeline(std::shared_ptr<Shader> &shader) :
        desc{ shader }
    {
        
    }

    virtual ~Pipeline() { }

    virtual void Map() { }

    virtual void Unmap() { }

    virtual void Bind(std::shared_ptr<Texture> &texture, uint32_t slot = 0)
    {

    }

    virtual void Bind(const std::string &name, const Buffer *uniform)
    {

    }

    virtual void Set(std::shared_ptr<Buffer> &buffer)
    {
        if (buffer->GetType() == Buffer::Type::Vertex)
        {
            desc.vertexBuffers.emplace_back(buffer);
        }
        if (buffer->GetType() == Buffer::Type::Index)
        {
            desc.indexBuffer = buffer;
        }
    }

    virtual void Set(const InputElementDescription &description)
    {
        desc.layout = description;
    }

    virtual void Create(const std::shared_ptr<RenderTarget> &renderTarget)
    {
        
    }

    virtual void Reconstruct(const std::shared_ptr<RenderTarget> &renderTarget)
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

        DrawType Type{ DrawType::Static };

        PrimitiveType PrimitiveType = PrimitiveType::Triangles;
    } desc;
};

using SuperPipeline = Pipeline;

}
