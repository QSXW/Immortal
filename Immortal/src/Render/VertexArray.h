#pragma once

#include "ImmortalCore.h"

#include "Buffer.h"

namespace Immortal {

	class IMMORTAL_API VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Map() const = 0;
		virtual void Unmap() const = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer) = 0;

		virtual const std::vector<Ref<VertexBuffer> > &GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer> &GetIndexBuffer() const = 0;

		static Ref<VertexArray> Create();
	};

}

