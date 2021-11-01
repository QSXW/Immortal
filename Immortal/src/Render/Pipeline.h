#pragma once

#include "ImmortalCore.h"

#include "Buffer.h"
#include "Shader.h"

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

    virtual void Set(std::shared_ptr<Buffer> &buffer)
    {

    }

    virtual void Set(const InputElementDescription &description)
    {
        desc.Layout = description;
    }

protected:
    struct Description
    {
        std::shared_ptr<Shader> Shader{ nullptr };

        std::vector<std::shared_ptr<Buffer>> vertexBuffers;

        std::shared_ptr<Buffer> indexBuffer;

        VertexLayout Layout{};

        DrawType Type{ DrawType::Static };

        PrimitiveType PrimitiveType = PrimitiveType::Triangles;
    } desc;
};

using SuperPipeline = Pipeline;

}
