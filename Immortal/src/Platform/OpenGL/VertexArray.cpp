#include "impch.h"
#include "VertexArray.h"

#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

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
    for (const auto &e : layout)
    {
        auto glBaseType = e.BaseType<GLenum>();
        glEnableVertexAttribArray(attribIndex);
        if (glBaseType == GL_INT)
        {
            glVertexAttribIPointer(attribIndex,
                e.ComponentCount(),
                glBaseType,
                layout.Stride(),
                (const void*)(intptr_t)e.Offset());
        }
        else
        {
            glVertexAttribPointer(attribIndex,
                e.ComponentCount(),
                glBaseType,
                GL_FALSE,
                layout.Stride(),
                (const void*)(intptr_t)e.Offset());
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
