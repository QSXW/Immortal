#pragma once
#include "ImmortalCore.h"
#include "Render/Buffer.h"

namespace Immortal
{
namespace OpenGL
{

class Buffer : public SuperBuffer
{
public:
    using Super         = SuperBuffer;
    using BindPointType = uint32_t;

public:
    Buffer(size_t size, Type type);

    Buffer(size_t size, const void *data, Type type);

    Buffer(size_t size, int binding);

    virtual ~Buffer() override;

    uint32_t Handle() const
    { 
        return handle;
    }

    operator uint32_t() const
    {
        return handle;
    }

    virtual void Update(uint32_t size, const void *data) override;

    void Bind() const
    {
        glBindBuffer(bindPoint, handle);
    }

    void Unbind() const
    {
        glBindBuffer(bindPoint, 0);
    }

    void SelectBindPoint(Type type)
    {
        if (type == Type::Index)
        {
            bindPoint = GL_ELEMENT_ARRAY_BUFFER;
        }
    }

private:
    uint32_t handle{};

    BindPointType bindPoint{ GL_ARRAY_BUFFER };
};

}
}
