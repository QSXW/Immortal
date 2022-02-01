#pragma once
#include "Core.h"
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

    Buffer(size_t size, uint32_t binding);

    virtual ~Buffer() override;

    virtual uint32_t Handle() const
    { 
        return handle;
    }

    virtual operator uint32_t() const
    {
        return handle;
    }

    virtual void Update(uint32_t size, const void *data, uint32_t offset = 0) override;

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
        switch (type)
        {
        case Type::Index:
            bindPoint = GL_ELEMENT_ARRAY_BUFFER;
            break;

        case Type::Uniform:
            bindPoint = GL_UNIFORM_BUFFER;
            break;

        case Type::Vertex:
        default:
            break;
        }
    }

protected:
    uint32_t handle{};

    BindPointType bindPoint{ GL_ARRAY_BUFFER };
};

class UniformBuffer : public Buffer
{
public:
    using Super = Buffer;

public:
    UniformBuffer(size_t size, uint32_t binding);

    GLuint Binding() const
    {
        return binding;
    }

    virtual void Update(uint32_t size, const void *data, uint32_t offset = 0) override;

private:
    uint32_t binding{ 0 };
};

}
}
