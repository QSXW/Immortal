#include "impch.h"
#include "VertexArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal
{
namespace OpenGL
{

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
    switch (type)
    {
        case Shader::DataType::Float:    
        case Shader::DataType::Float2:   
        case Shader::DataType::Float3:   
        case Shader::DataType::Float4:   
        case Shader::DataType::Mat3:     
        case Shader::DataType::Mat4:     return GL_FLOAT;
        case Shader::DataType::Int: 
        case Shader::DataType::Int2:
        case Shader::DataType::Int3:
        case Shader::DataType::Int4:     return GL_INT;
        case Shader::DataType::Bool:     return GL_BOOL;
    }

    SLASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &handle);
    glBindVertexArray(handle);
    glBindVertexArray(0);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &handle);
}

void VertexArray::Map() const
{
    glBindVertexArray(handle);
}

void VertexArray::Unmap() const
{
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
    vertexBuffer->Map();
    glBindVertexArray(handle);

    const auto& layout = vertexBuffer->Layout();
    uint32_t attribIndex = 0;
    for (const auto& element : layout)
    {
        auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
        glEnableVertexAttribArray(attribIndex);
        if (glBaseType == GL_INT)
        {
            glVertexAttribIPointer(attribIndex,
                element.GetComponentCount(),
                glBaseType,
                layout.Stride(),
                (const void*)(intptr_t)element.Offset);
        }
        else
        {
            glVertexAttribPointer(attribIndex,
                element.GetComponentCount(),
                glBaseType,
                element.Normalized ? GL_TRUE : GL_FALSE,
                layout.Stride(),
                (const void*)(intptr_t)element.Offset);
        }
        attribIndex++;
    }
    vertexBuffer->Unmap();
    buffers.push_back(vertexBuffer);
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<SuperIndexBuffer> &indexBuffer)
{
    indexBuffer->Map();
    glBindVertexArray(handle);
    glVertexArrayElementBuffer(handle, indexBuffer->Handle());
    indexBuffer->Unmap();
    this->indexBuffer = indexBuffer;
}

}
}
