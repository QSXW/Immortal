#pragma once
#include "ImmortalCore.h"
#include "Render/Buffer.h"

namespace Immortal
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(const void *vertices, uint32_t size);
		~OpenGLVertexBuffer() override;

		uint32_t Handle() const override { return mHandle; }

		void Map() const override;
		void Unmap() const override;

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
		uint32_t mHandle{};
		VertexLayout mLayout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const void *indices, uint32_t count);
		~OpenGLIndexBuffer();

		uint32_t Handle() const override { return mHandle; }
		void Map() const override;
		void Unmap() const override;

		virtual uint32_t Count() const override
		{
			return mCount;
		}
	
	private:
		uint32_t mHandle{};
		uint32_t mCount;
	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(size_t size, int binding);
		~OpenGLUniformBuffer();

		virtual void SetData(size_t size, const void *data) const override;
		void Map() const;
		void Unmap() const override;

	private:
		uint32_t mHandle{};
	};

}
