#pragma once

#include "Core.h"
#include "Buffer.h"

namespace Immortal
{
namespace OpenGL
{

class VertexArray
{
public:
	GLenum GetBaseType(GLenum format)
    {
		switch (format)
        {
			case GL_R32F:
			case GL_RG32F:
			case GL_RGB32F:
			case GL_RGBA32F:
                return GL_FLOAT;
			case GL_INT:
			case GL_R32I:
			case GL_RG32I:
			case GL_RGB32I:
			case GL_RGBA32I:
				return GL_INT;
			default:
				return format;
        }
    }

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

    void Set(const Buffer *buffer, const InputElementDescription &inputElementDescription)
    {
        Bind();
        buffer->Bind();
        uint32_t attributeIndex = 0;
        for (const auto &e : inputElementDescription)
        {
			auto glBaseType = GetBaseType(e.BaseType<GLenum>());
            if (glBaseType == GL_INT)
            {
                glVertexAttribIPointer(attributeIndex,
                    e.ComponentCount(),
                    glBaseType,
                    inputElementDescription.Stride,
                    (const void*)(intptr_t)e.Offset());
            }
            else
            {
                glVertexAttribPointer(attributeIndex,
                    e.ComponentCount(),
                    glBaseType,
                    GL_FALSE,
                    inputElementDescription.Stride,
                    rcast<const void *>((intptr_t)e.Offset()));
            }
            glEnableVertexAttribArray(attributeIndex++);
        }
        buffer->Unbind();
        Unbind();
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
