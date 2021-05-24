#pragma once
#include "ImmortalCore.h"
#include "Immortal/Render/Buffer.h"

namespace Immortal
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(const void *vertices, uint32_t size);
		~OpenGLVertexBuffer() override;

		__forceinline uint32_t RendererID() const override { return mRendererID; }

		__forceinline void Bind() const override;
		__forceinline void UnBind() const override;

		void SetLayout(const VertexLayout &layout) override
		{
			mLayout = layout;
		}

		const VertexLayout &Layout() const override
		{ 
			return mLayout;
		}

		void SetData(const void *data, uint32_t size) override;

	private:
		uint32_t mRendererID{};
		VertexLayout mLayout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const void *indices, uint32_t count);
		~OpenGLIndexBuffer();

		__forceinline uint32_t RendererID() const override { return mRendererID; }
		__forceinline void Bind() const override;
		__forceinline void UnBind() const override;

		__forceinline virtual uint32_t Count() const override
		{
			return mCount;
		}
	
	private:
		uint32_t mRendererID{};
		uint32_t mCount;
	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(size_t size, int binding);
		~OpenGLUniformBuffer();

		virtual void SetData(size_t size, const void *data) const override;
		void Bind() const;
		void UnBind() const override;

	private:
		uint32_t mRendererID{};
	};

}


