#pragma once

#include "ImmortalCore.h"

#include "Buffer.h"

namespace Immortal
{

class IMMORTAL_API VertexArray
{
public:
    virtual ~VertexArray() = default;

    virtual void Map() const = 0;
    virtual void Unmap() const = 0;

    virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) = 0;

    virtual const std::vector<std::shared_ptr<VertexBuffer> > &GetVertexBuffers() const = 0;
    virtual const std::shared_ptr<IndexBuffer> &GetIndexBuffer() const = 0;

    static std::shared_ptr<VertexArray> Create();
};

using SuperVertexArray = VertexArray;
}

