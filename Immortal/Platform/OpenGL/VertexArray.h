#pragma once

#include "Core.h"
#include "Buffer.h"
#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

class VertexArray : public Handle
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
        
            case GL_FORMAT_RGBA8:
				return GL_FORMAT_UNSIGNED_BYTE;

			default:
				return format;
        }
    }

    
    GLCPP_SWAPPABLE(VertexArray)

public:
    VertexArray() :
        Handle{}
    {

    }

	VertexArray(const InputElementDescription &inputElementDescription) :
	    Handle{},
	    inputElementDescription{ inputElementDescription }
    {
        glCreateVertexArrays(1, &handle);
        glBindVertexArray(handle);
        glBindVertexArray(0);
    }

    ~VertexArray()
    {
        if (handle)
        {
			glDeleteVertexArrays(1, &handle);
			handle = 0;
        }
    }

    void Bind() const
    {
        glBindVertexArray(handle);
    }

    void Unbind() const
    {
        glBindVertexArray(0);
    }

    void Activate(Buffer *vertexBuffer)
    {
        Bind();
		vertexBuffer->Bind();
        uint32_t attributeIndex = 0;
        for (const auto &inputElement : inputElementDescription)
        {
			GL_FORMAT glBaseType = GetBaseType(inputElement.format);
			if (glBaseType == GL_FORMAT_INT)
            {
                glVertexAttribIPointer(attributeIndex,
                    inputElement.ComponentCount(),
                    glBaseType,
                    inputElementDescription.GetStride(),
                    (const void*)(intptr_t)inputElement.GetOffset()
                );
            }
            else
            {
				bool nomalize = GL_FALSE;
                if (inputElement.format == Format::RGBA8)
                {
					nomalize = GL_TRUE;
                }
                glVertexAttribPointer(attributeIndex,
                    inputElement.ComponentCount(),
                    glBaseType,
					nomalize,
                    inputElementDescription.GetStride(),
                    (const void *)((intptr_t)inputElement.GetOffset()));
            }
            glEnableVertexAttribArray(attributeIndex++);
        }
		vertexBuffer->Unbind();
        Unbind();
    }

    void Swap(VertexArray &other)
    {
		std::swap(handle, other.handle);
		std::swap(inputElementDescription, other.inputElementDescription);
    }

protected:
	InputElementDescription inputElementDescription;
};

}
}
