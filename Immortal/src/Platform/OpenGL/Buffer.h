#pragma once
#include "ImmortalCore.h"
#include "Render/Buffer.h"

namespace Immortal
{
namespace OpenGL
{

class VertexBuffer : public SuperVertexBuffer
{
public:
    VertexBuffer(uint32_t size);

    VertexBuffer(const void *vertices, uint32_t size);

    ~VertexBuffer() override;

    uint32_t Handle() const override
    { 
        return handle;
    }

    void Map() const override;
    void Unmap() const override;

    void SetLayout(const VertexLayout &other) override
    {
        layout = other;
    }

    const VertexLayout &Layout() const override
    { 
        return layout;
    }

    void SetData(const void *data, uint32_t size) override;

private:
    uint32_t handle{};

    VertexLayout layout;
};

class IndexBuffer : public SuperIndexBuffer
{
public:
    IndexBuffer(const void *indices, uint32_t count);
    ~IndexBuffer();

    uint32_t Handle() const override 
    { 
        return handle;
    }

    void Map() const override;

    void Unmap() const override;

    virtual uint32_t Count() const override
    {
        return count;
    }
    
private:
    uint32_t handle{};

    uint32_t count;
};

class UniformBuffer : public SuperUniformBuffer
{
public:
    UniformBuffer(size_t size, int binding);

    ~UniformBuffer();

    virtual void SetData(size_t size, const void *data) const override;

    void Map() const;

    void Unmap() const override;

private:
    uint32_t handle{};
};

}
}
