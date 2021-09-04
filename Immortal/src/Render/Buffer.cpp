#include "impch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Immortal
{
Ref<VertexBuffer> VertexBuffer::Create(const void * vertices, UINT32 size)
{
	return InstantiateGrphicsPrimitive<VertexBuffer, OpenGLVertexBuffer, OpenGLVertexBuffer, OpenGLVertexBuffer>(vertices, size);
}

Ref<VertexBuffer> VertexBuffer::Create(UINT32 size)
{
	return InstantiateGrphicsPrimitive<VertexBuffer, OpenGLVertexBuffer, OpenGLVertexBuffer, OpenGLVertexBuffer>(size);
}

Ref<IndexBuffer> IndexBuffer::Create(const void * indices, UINT32 count)
{
	return InstantiateGrphicsPrimitive<IndexBuffer, OpenGLIndexBuffer, OpenGLIndexBuffer, OpenGLIndexBuffer>(indices, count);
}

Ref<UniformBuffer> UniformBuffer::Create(size_t size, int binding)
{
	return InstantiateGrphicsPrimitive<UniformBuffer, OpenGLUniformBuffer, OpenGLUniformBuffer, OpenGLUniformBuffer>(size, binding);
}
}