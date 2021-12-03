#pragma once

#include "ImmortalCore.h"
#include "Buffer.h"

namespace Immortal
{
namespace OpenGL
{

class VertexArray
{
public:
    VertexArray()
    {
        glCreateVertexArrays(1, &handle);
        glBindVertexArray(handle);
        glBindVertexArray(0);
    }

    ~VertexArray()
    {
        glDeleteVertexArrays(1, &handle);
    }

    void Bind() const
    {
        glBindVertexArray(handle);
    }

    void Unbind() const
    {
        glBindVertexArray(0);
    }

    void Set(const InputElementDescription &inputElementDescription)
    {
        glBindVertexArray(handle);

        uint32_t attributeIndex = 0;
        for (const auto &e : inputElementDescription)
        {
            auto glBaseType = e.BaseType<GLenum>();
            glEnableVertexAttribArray(attributeIndex);
            if (glBaseType == GL_INT)
            {
                glVertexAttribIPointer(attributeIndex,
                    e.ComponentCount(),
                    glBaseType,
                    inputElementDescription.Stride(),
                    (const void*)(intptr_t)e.Offset());
            }
            else
            {
                glVertexAttribPointer(attributeIndex,
                    e.ComponentCount(),
                    glBaseType,
                    GL_FALSE,
                    inputElementDescription.Stride(),
                    rcast<const void *>((intptr_t)e.Offset()));
            }
            attributeIndex++;
        }
    }

    void Bind(const Buffer *buffer)
    {
        Bind();
        buffer->Bind();
        if (buffer->GetType() == Buffer::Type::Vertex)
        {

        }
        else if (buffer->GetType() == Buffer::Type::Index)
        {
            glVertexArrayElementBuffer(handle, *buffer);
        }
        buffer->Unbind();
        Unbind();
    }

private:
    GLuint handle;
};

}
}
