#include "impch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal {

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

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &mRendererID);
		glBindVertexArray(mRendererID);
		glBindVertexArray(0);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &mRendererID);
	}

	void OpenGLVertexArray::Map() const
	{
		glBindVertexArray(mRendererID);
	}

	void OpenGLVertexArray::UnMap() const
	{
		glBindVertexArray(0);
	}
	/*
	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		SLASSERT(vertexBuffer->Layout().Elements().size(), "Vertex Buffer has no layout!");
		glBindVertexArray(mRendererID);

		vertexBuffer->Map();

		uint32_t index = 0;
		const auto &layout = vertexBuffer->Layout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized,
				layout.Stride(),
				reinterpret_cast<const void*>(element.Offset));
			index++;
		}

		mVertexBuffers.push_back(vertexBuffer);
	}*/


	// @Fixed a bug occurred above function:
	//  Float type vertex data should use glVertexAttribPointer,
	//  and Int type should use glVertexAttribIPointer
	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		vertexBuffer->Map();
		glBindVertexArray(mRendererID);

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
		vertexBuffer->UnMap();
		mVertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		indexBuffer->Map();
		glBindVertexArray(mRendererID);
		glVertexArrayElementBuffer(mRendererID, indexBuffer->RendererID());
		indexBuffer->UnMap();
		mIndexBuffer = indexBuffer;
	}
}