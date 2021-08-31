#include "impch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal {

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		glCreateBuffers(1, &mRendererID);
		glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(const void * vertices, uint32_t size)
	{
		glCreateBuffers(1, &mRendererID);
		glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &mRendererID);
	}

	void OpenGLVertexBuffer::Map() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
	}

	void OpenGLVertexBuffer::UnMap() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	/* Use to submit data in real-time */
	void OpenGLVertexBuffer::SetData(const void * data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(const void *indices, uint32_t size)
		: mCount(size / sizeof(uint32_t))
	{
		glCreateBuffers(1, &mRendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &mRendererID);
	}

	void OpenGLIndexBuffer::Map() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
	}

	void OpenGLIndexBuffer::UnMap() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	OpenGLUniformBuffer::OpenGLUniformBuffer(size_t size, int binding)
	{
		glCreateBuffers(1, &mRendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, mRendererID);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, binding, mRendererID, 0, size);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &mRendererID);
	}

	__forceinline void OpenGLUniformBuffer::Map() const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, mRendererID);
	}

	void OpenGLUniformBuffer::SetData(size_t size, const void *data) const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, mRendererID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLUniformBuffer::UnMap() const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

}

