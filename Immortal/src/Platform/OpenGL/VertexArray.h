#pragma once

#include "ImmortalCore.h"
#include "Render/VertexArray.h"

namespace Immortal
{
namespace OpenGL
{

class VertexArray : public SuperVertexArray
{
public:
    VertexArray();

    ~VertexArray();

    void Map() const;

    void Unmap() const;

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) override;

    void SetIndexBuffer(const std::shared_ptr<SuperIndexBuffer> &indexBuffer) override;

    const std::vector<std::shared_ptr<VertexBuffer> > &GetVertexBuffers() const override
    {
        return buffers;
    }

    const std::shared_ptr<IndexBuffer> &GetIndexBuffer() const override
    {
        return indexBuffer;
    }

private:
    uint32_t handle;

    uint32_t index = 0;

    std::vector<std::shared_ptr<VertexBuffer> > buffers;

    std::shared_ptr<IndexBuffer> indexBuffer;
};

}
}
