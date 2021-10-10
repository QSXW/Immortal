#pragma once

#include "ImmortalCore.h"

#include "Shader.h"

namespace Immortal
{

static UINT32 ShaderDataTypeSize(Shader::DataType type)
{
    switch (type)
    {
        case Shader::DataType::Float:    return sizeof(float);
        case Shader::DataType::Float2:   return sizeof(float) * 2;
        case Shader::DataType::Float3:   return sizeof(float) * 3;
        case Shader::DataType::Float4:   return sizeof(float) * 4;
        case Shader::DataType::Mat3:     return sizeof(float) * 3 * 3;
        case Shader::DataType::Mat4:     return sizeof(float) * 4 * 4;
        case Shader::DataType::Int:      return sizeof(int);
        case Shader::DataType::Int2:     return sizeof(int) * 2;
        case Shader::DataType::Int3:     return sizeof(int) * 3;
        case Shader::DataType::Int4:     return sizeof(int) * 4;
        case Shader::DataType::Bool:     return sizeof(bool);
    }

    SLASSERT(false && "Unknown ShaderDataType!");
    return 0;
}

struct InputElementDescription
{
    std::string Name;
    Shader::DataType Type;
    UINT32 Size;
    size_t Offset;
    bool Normalized;

    InputElementDescription() = default;

    InputElementDescription(Shader::DataType type, const std::string& name, bool normalized = false)
        : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
    {

    }

    uint32_t GetComponentCount() const
    {
        switch (Type)
        {
            case ShaderDataType::Float:   return 1;
            case ShaderDataType::Float2:  return 2;
            case ShaderDataType::Float3:  return 3;
            case ShaderDataType::Float4:  return 4;
            case ShaderDataType::Mat3:    return 3 * 3;
            case ShaderDataType::Mat4:    return 4 * 4;
            case ShaderDataType::Int:     return 1;
            case ShaderDataType::Int2:    return 2;
            case ShaderDataType::Int3:    return 3;
            case ShaderDataType::Int4:    return 4;
            case ShaderDataType::Bool:    return 1;
        }

        SLASSERT(false && "Unknown ShaderDataType!");
        return 0;
    }

};

class VertexLayout
{
public:
    VertexLayout() = default;
    VertexLayout(std::initializer_list<InputElementDescription> elements)
        : mElementDescriptions(elements)
    {
        this->CalculateOffsetAndStride();
    }

    UINT32 Stride() const { return mStride; }
    const std::vector<InputElementDescription>& Elements() const { return mElementDescriptions;  }

    std::vector<InputElementDescription>::iterator begin() { return mElementDescriptions.begin(); }
    std::vector<InputElementDescription>::iterator end() { return mElementDescriptions.end(); }
    std::vector<InputElementDescription>::const_iterator begin() const { return mElementDescriptions.begin(); }
    std::vector<InputElementDescription>::const_iterator end() const { return mElementDescriptions.end(); }

private:
    void CalculateOffsetAndStride()
    {
        size_t offset = 0;
        mStride = 0;

        for (auto &element : mElementDescriptions)
        {
            element.Offset = offset;
            offset += element.Size;
            mStride += element.Size;
        }
    }

private:
    std::vector<InputElementDescription> mElementDescriptions;
    UINT32 mStride = 0;
};

class IMMORTAL_API VertexBuffer
{
public:
    virtual ~VertexBuffer() = default;

    virtual uint32_t Handle() const = 0;

    virtual void Map() const  = 0;
    virtual void Unmap() const = 0;

    virtual void SetLayout(const VertexLayout &layout) = 0;
    virtual const VertexLayout &Layout() const = 0;

    virtual void SetData(const void *data, uint32_t size) = 0;

    static std::shared_ptr<VertexBuffer> Create(uint32_t size);
    static std::shared_ptr<VertexBuffer> Create(const void *vertices, uint32_t size);
};

using SuperVertexBuffer = VertexBuffer;

class IMMORTAL_API IndexBuffer
{
public:
    virtual ~IndexBuffer() = default;

    virtual uint32_t Handle() const = 0;

    virtual void Map() const = 0;
    virtual void Unmap() const = 0;

    virtual uint32_t Count() const = 0;

    static std::shared_ptr<IndexBuffer> Create(const void *indices, uint32_t count);
};

using SuperIndexBuffer = IndexBuffer;

class IMMORTAL_API UniformBuffer
{
public:
    virtual ~UniformBuffer() = default;

    virtual void SetData(size_t size, const void *data) const = 0;

    virtual void Unmap() const = 0;

    static std::shared_ptr<UniformBuffer> Create(size_t size, int biding = 0);
};

using SuperUniformBuffer = UniformBuffer;

}
