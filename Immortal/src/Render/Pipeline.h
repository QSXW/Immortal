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
        Line,
        Triangles
    };

public:
    Pipeline() { }

    virtual ~Pipeline() { }

    virtual void Map()   const = 0;

    virtual void Unmap() const = 0;

private:
    struct Description
    {
        VertexLayout Layout{};

        DrawType Type{ DrawType::Static };

        std::shared_ptr<Shader> Shader{};

        struct DepthStencilState
        {
            bool DepthEnable = false;
            bool StencilEnable = false;
        };

        UINT32 SampleMask = UINT_MAX;

        PrimitiveType PrimitiveType = PrimitiveType::Triangles;
    } description;
};

}
