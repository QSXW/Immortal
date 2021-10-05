#pragma once

#include "ImmortalCore.h"
#include "Render/VertexArray.h"

namespace Immortal
{

class OpenGLVertexArray : public VertexArray
{
public:
    OpenGLVertexArray();
    ~OpenGLVertexArray();

    void Map() const;
    void Unmap() const;

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) override;
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) override;

    const std::vector<std::shared_ptr<VertexBuffer> > &GetVertexBuffers() const override
    {
        return mVertexBuffers;
    }

    const std::shared_ptr<IndexBuffer> &GetIndexBuffer() const override
    {
        return mIndexBuffer;
    }

private:
    uint32_t mHandle;
    uint32_t mVertexBufferIndex = 0;
    std::vector<std::shared_ptr<VertexBuffer> > mVertexBuffers;
    std::shared_ptr<IndexBuffer> mIndexBuffer;
};

}
