#pragma once

#include "ImmortalCore.h"
#include "Shader.h"

#include "Platform/OpenGL/Common.h"
#include "Platform/Vulkan/Common.h"
#include "Platform/D3D12/Common.h"

#include <array>

namespace Immortal
{

/**
  * @brief: FLOAT here represents 32-bits floating points
  *         INT represents 32-bits signed integers
  */
enum class Format
{
    INT,
    IVECTOR2,
    IVECTOR3,
    IVECTOR4,
    FLOAT,
    VECTOR2,
    VECTOR3,
    VECTOR4,
    COLOUR = VECTOR4,
    MATRIX4,
    R8G8B8A8_UNORM = 0,
    R8G8B8A8_SRGB  = 1,
    UNDEFINED      = -1
};

struct BaseFormatElement
{
    BaseFormatElement(VkFormat vkFormat, DXGI_FORMAT dxFormat, GLenum glFormat, uint32_t size, uint32_t count) :
        Vulkan{ vkFormat },
        DXGI{ dxFormat },
        OpenGL{ glFormat },
        size{ size },
        componentCount{ count }
    {
        
    }

    VkFormat    Vulkan;
    DXGI_FORMAT DXGI;
    GLenum      OpenGL;
    uint32_t    size;
    uint32_t    componentCount;
};

static BaseFormatElement BaseFormatMapper[] = {
    { VK_FORMAT_R32_SINT,            DXGI_FORMAT_R32_SINT,           GL_INT,   sizeof(int32_t),     1     },
    { VK_FORMAT_R32G32_SINT,         DXGI_FORMAT_R32G32_SINT,        GL_INT,   sizeof(int32_t) * 2, 2     },
    { VK_FORMAT_R32G32B32_SINT,      DXGI_FORMAT_R32G32B32_SINT,     GL_INT,   sizeof(int32_t) * 3, 3     },
    { VK_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_R32G32B32A32_SINT,  GL_INT,   sizeof(int32_t) * 4, 4     },
    { VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,          GL_FLOAT, sizeof(float),       1     },
    { VK_FORMAT_R32G32_SFLOAT,       DXGI_FORMAT_R32G32_FLOAT,       GL_FLOAT, sizeof(Vector2),     2     },
    { VK_FORMAT_R32G32B32_SFLOAT,    DXGI_FORMAT_R32G32B32_FLOAT,    GL_FLOAT, sizeof(Vector3),     3     },
    { VK_FORMAT_R32G32B32A32_SFLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, GL_FLOAT, sizeof(Vector4),     4     },
    { VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,          GL_FLOAT, sizeof(Matrix4),     4 * 4 }
};

struct InputElement
{
    InputElement()
    {
    
    }

    InputElement(Format foramt, const std::string &name) :
        foramt{ foramt },
        name{ name }
    {

    }

    template <class T>
    T BaseType() const
    {
        if constexpr (is_same<T, GLenum>())
        {
            return BaseFormatMapper[ncast<int>(foramt)].OpenGL;
        }
        if constexpr (is_same<T, VkFormat>())
        {
            return BaseFormatMapper[ncast<int>(foramt)].Vulkan;
        }
        if constexpr (is_same<T, DXGI_FORMAT>())
        {
            return BaseFormatMapper[ncast<int>(foramt)].DXGI;
        }
    }

    uint32_t ComponentCount() const
    {
        return BaseFormatMapper[ncast<int>(foramt)].componentCount;
    }

    uint32_t Offset() const
    {
        return offset;
    }

    std::string name;
    uint32_t    offset{ 0 };
    Format      foramt{ Format::UNDEFINED };
};

struct InputElementDescription
{
    InputElementDescription()
    {
    
    }

    InputElementDescription(const InputElementDescription &other) :
        elements{ other.elements.begin(), other.elements.end() }
    {

    }

    InputElementDescription(InputElementDescription &&other) :
        elements{ std::move(other.elements) }
    {

    }

    InputElementDescription &operator=(const InputElementDescription &other)
    {
        elements.resize(other.elements.size());
        CopyProps(elements.data(), other.elements.data());
        return *this;
    }

    InputElementDescription operator=(InputElementDescription &&other)
    {
        elements.swap(other.elements);
        return *this;
    }

    InputElementDescription(std::initializer_list<InputElement> &&list) :
        elements{ std::move(list) }
    {
        INIT();
    }

    auto begin()
    {
        return elements.begin();
    }

    auto end()
    {
        return elements.end();
    }

    auto begin() const
    {
        return elements.cbegin();
    }

    auto end() const
    {
        return elements.cend();
    }

    auto operator[](size_t index) const
    {
        return elements[index];
    }

    auto Size() const
    {
        return elements.size();
    }

    uint32_t Stride() const
    {
        return stride;
    }

private:
    void INIT()
    {
        uint32_t offset = 0;
        for (auto &e : elements)
        {
            e.offset =  offset;
            offset   += BaseFormatMapper[ncast<int>(e.foramt)].size;
        }
        stride = offset;
    }

    std::vector<InputElement> elements;

    uint32_t stride{ 0 };
};

using VertexLayout = InputElementDescription;

class Buffer
{
public:
    enum class Type
    {
        Layout,
        Vertex,
        Index,
        Uniform,
        PushConstant
    };
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
