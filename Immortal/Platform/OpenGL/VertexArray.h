#pragma once

#include "Core.h"
#include "Buffer.h"
#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

class VertexArray
{
public:
	GL_FORMAT GetBaseType(Format _format)
    {
		GL_FORMAT format = _format;
		switch (format)
        {
			case GL_FORMAT_R32F:
			case GL_FORMAT_RG32F:
			case GL_FORMAT_RGB32F:
			case GL_FORMAT_RGBA32F:
                return GL_FORMAT_FLOAT;

			case GL_FORMAT_INT:
			case GL_FORMAT_R32I:
			case GL_FORMAT_RG32I:
			case GL_FORMAT_RGB32I:
			case GL_FORMAT_RGBA32I:
				return GL_FORMAT_INT;

			default:
				return format;
        }
    }

    GLCPP_OPERATOR_HANDLE()

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
			GL_FORMAT glBaseType = GetBaseType(e.format);
			if (glBaseType == GL_FORMAT_INT)
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
};

}
}
