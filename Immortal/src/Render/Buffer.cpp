#include "impch.h"
#include "Buffer.h"

#include "Render.h"
#include "Platform/OpenGL/Buffer.h"

namespace Immortal
{

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const void * vertices, UINT32 size)
{
    return CreateSuper<VertexBuffer, OpenGL::VertexBuffer, OpenGL::VertexBuffer, OpenGL::VertexBuffer>(vertices, size);
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(UINT32 size)
{
    return CreateSuper<VertexBuffer, OpenGL::VertexBuffer, OpenGL::VertexBuffer, OpenGL::VertexBuffer>(size);
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create(const void * indices, UINT32 count)
{
    return CreateSuper<IndexBuffer, OpenGL::IndexBuffer, OpenGL::IndexBuffer, OpenGL::IndexBuffer>(indices, count);
}

std::shared_ptr<UniformBuffer> UniformBuffer::Create(size_t size, int binding)
{
    return CreateSuper<UniformBuffer, OpenGL::UniformBuffer, OpenGL::UniformBuffer, OpenGL::UniformBuffer>(size, binding);
}

}
