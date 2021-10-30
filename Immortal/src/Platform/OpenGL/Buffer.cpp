#include "impch.h"
#include "Buffer.h"

#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

VertexBuffer::VertexBuffer(uint32_t size)
{
    glCreateBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::VertexBuffer(const void * vertices, uint32_t size)
{
    glCreateBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &handle);
}

void VertexBuffer::Map() const
{
    glBindBuffer(GL_ARRAY_BUFFER, handle);
}

void VertexBuffer::Unmap() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetData(const void * data, uint32_t size)
{
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

IndexBuffer::IndexBuffer(const void *indices, uint32_t size)
    : count(size / sizeof(uint32_t))
{
    glCreateBuffers(1, &handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &handle);
}

void IndexBuffer::Map() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
}

void IndexBuffer::Unmap() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

UniformBuffer::UniformBuffer(size_t size, int binding)
{
    glCreateBuffers(1, &handle);
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, binding, handle, 0, size);
}

UniformBuffer::~UniformBuffer()
{
    glDeleteBuffers(1, &handle);
}

void UniformBuffer::Map() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
}

void UniformBuffer::SetData(size_t size, const void *data) const
{
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::Unmap() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

}
}
