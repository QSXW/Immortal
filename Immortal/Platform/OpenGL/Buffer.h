#pragma once
#include "Core.h"
#include "Common.h"
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

	GLCPP_OPERATOR_HANDLE()

public:
    Buffer(uint64_t size, Type type);

    Buffer(uint64_t size, const void *data, Type type);

    Buffer(uint64_t size, uint32_t binding);

    virtual ~Buffer() override;

    virtual void Update(uint64_t size, const void *data, uint64_t offset = 0) override;

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
    BindPointType bindPoint{ GL_ARRAY_BUFFER };
};

class UniformBuffer : public Buffer
{
public:
    using Super = Buffer;

public:
    UniformBuffer(uint64_t size, uint32_t binding);

    GLuint Binding() const
    {
        return binding;
    }

    virtual void Update(uint64_t size, const void *data, uint64_t offset = 0) override;

private:
    uint32_t binding{ 0 };
};

}
}
