#include "impch.h"
#include "Buffer.h"

#include "Render.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Immortal
{
std::shared_ptr<VertexBuffer> VertexBuffer::Create(const void * vertices, UINT32 size)
{
	return CreateSuper<VertexBuffer, OpenGLVertexBuffer, OpenGLVertexBuffer, OpenGLVertexBuffer>(vertices, size);
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(UINT32 size)
{
	return CreateSuper<VertexBuffer, OpenGLVertexBuffer, OpenGLVertexBuffer, OpenGLVertexBuffer>(size);
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create(const void * indices, UINT32 count)
{
	return CreateSuper<IndexBuffer, OpenGLIndexBuffer, OpenGLIndexBuffer, OpenGLIndexBuffer>(indices, count);
}

std::shared_ptr<UniformBuffer> UniformBuffer::Create(size_t size, int binding)
{
	return CreateSuper<UniformBuffer, OpenGLUniformBuffer, OpenGLUniformBuffer, OpenGLUniformBuffer>(size, binding);
}
}