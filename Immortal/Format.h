#pragma once

#include "Core.h"
#include <glad/glad.h>
#include <vulkan/vulkan_core.h>
#include <dxgiformat.h>

namespace Immortal
{

/**
 * @brief: FLOAT here represents 32-bits floating points
 *         INT represents 32-bits signed integers
 */
struct Format
{
public:
    enum ValueType : uint32_t
    {
        R8G8B8A8_UNORM = 0,
        R8G8B8A8_SRGB = 1,
        INT = 0,
        IVECTOR2,
        IVECTOR3,
        IVECTOR4,
        FLOAT,
        VECTOR2,
        VECTOR3,
        VECTOR4,
        COLOUR = VECTOR4,
        MATRIX4,
        R8,
        R16,
        R16F,
        R32,
        R32F,
        RG8,
        RG16,
        RG16F,
        RG32,
        RG32F,
        RGB8,
        RGB32,
        RGB32F,
        RGBA8,
        RGBA16,
        RGBA16F,
        RGBA32,
        RGBA32F,
        BGRA8,
        SRGB,
        Depth32F,
        Depth24Stencil8,
        YUV420P,
        YUV422P,
        YUV444P,
        Depth = Depth32F,
        None
    };

    Format() :
        v{ None }
    {

    }

    Format(ValueType v) :
        v{ v }
    {

    }

    operator ValueType() const
    {
        return v;
    }

    size_t ComponentCount() const;

    size_t Size() const;

    operator VkFormat() const;

    operator DXGI_FORMAT() const;

    operator GLenum() const;

private:
    ValueType v;
};

namespace StaticSpace
{

struct FormatElement
{
    FormatElement(Format format, VkFormat vkFormat, DXGI_FORMAT dxFormat, GLenum glFormat, uint32_t size, uint32_t count) :
        Vulkan{ vkFormat },
        DXGI{ dxFormat },
        OpenGL{ glFormat },
        Size{ size },
        ComponentCount{ count }
    {
        
    }

    operator VkFormat() const
    {
        return Vulkan;
    }

    operator DXGI_FORMAT() const
    {
        return DXGI;
    }

    operator GLenum() const
    {
        return OpenGL;
    }

    VkFormat    Vulkan;
    DXGI_FORMAT DXGI;
    GLenum      OpenGL;
    uint32_t    Size;
    uint32_t    ComponentCount;
};

using bfloat = uint16_t;
#define FS_C(T, CC) (sizeof(T) * (CC)), CC

static inline FormatElement FromatElementTable[] = {
    { Format::INT,             VK_FORMAT_R32_SINT,            DXGI_FORMAT_R32_SINT,            GL_INT,               FS_C(int,      1)     },
    { Format::IVECTOR2,        VK_FORMAT_R32G32_SINT,         DXGI_FORMAT_R32G32_SINT,         GL_INT,               FS_C(int,      2)     },
    { Format::IVECTOR3,        VK_FORMAT_R32G32B32_SINT,      DXGI_FORMAT_R32G32B32_SINT,      GL_INT,               FS_C(int,      3)     },
    { Format::IVECTOR4,        VK_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_R32G32B32A32_SINT,   GL_INT,               FS_C(int,      4)     },
    { Format::FLOAT,           VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,           GL_FLOAT,             FS_C(float,    1)     },
    { Format::VECTOR2,         VK_FORMAT_R32G32_SFLOAT,       DXGI_FORMAT_R32G32_FLOAT,        GL_FLOAT,             FS_C(float,    2)     },
    { Format::VECTOR3,         VK_FORMAT_R32G32B32_SFLOAT,    DXGI_FORMAT_R32G32B32_FLOAT,     GL_FLOAT,             FS_C(float,    3)     },
    { Format::VECTOR4,         VK_FORMAT_R32G32B32A32_SFLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,  GL_FLOAT,             FS_C(float,    4)     },
    { Format::MATRIX4,         VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,           GL_FLOAT,             FS_C(float,    4 * 4) },
    { Format::R8,              VK_FORMAT_R8_UNORM,            DXGI_FORMAT_R8_UNORM,            GL_R8,                FS_C(uint8_t,  1)     },
    { Format::R16,             VK_FORMAT_R16_UNORM,           DXGI_FORMAT_R16_UNORM,           GL_R16,               FS_C(uint16_t, 1)     },
    { Format::R16F,            VK_FORMAT_R16_SFLOAT,          DXGI_FORMAT_R16_FLOAT,           GL_R32F,              FS_C(bfloat,   1)     },
    { Format::R32,             VK_FORMAT_R32_SINT,            DXGI_FORMAT_R32_SINT,            GL_INT ,              FS_C(uint32_t, 1)     },
    { Format::R32F,            VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,           GL_R32F,              FS_C(float,    1)     },
    { Format::RG8,             VK_FORMAT_R8G8_UNORM,          DXGI_FORMAT_R8G8_UNORM,          GL_RG8,               FS_C(uint8_t,  2)     },
    { Format::RG16,            VK_FORMAT_R16G16_UNORM,        DXGI_FORMAT_R16G16_UNORM,        GL_RG16,              FS_C(uint16_t, 2)     },
    { Format::RG16F,           VK_FORMAT_R16G16_SFLOAT,       DXGI_FORMAT_R16G16_FLOAT,        GL_RG32F,             FS_C(bfloat,   2)     },
    { Format::RG32,            VK_FORMAT_R32G32_SINT,         DXGI_FORMAT_R32G32_SINT,         GL_RG32I ,            FS_C(uint32_t, 2)     },
    { Format::RG32F,           VK_FORMAT_R32G32_SFLOAT,       DXGI_FORMAT_R32G32_FLOAT,        GL_RG32F,             FS_C(float,    2)     },
    { Format::RGB8,            VK_FORMAT_R8G8B8_UNORM,         DXGI_FORMAT_UNKNOWN,            GL_RGB8 ,             FS_C(uint8_t,  3)     },
    { Format::RGB32,           VK_FORMAT_R32G32B32_SINT,      DXGI_FORMAT_R32G32B32_SINT,      GL_RGB32I ,           FS_C(uint32_t, 3)     },
    { Format::RGB32F,          VK_FORMAT_R32G32B32_SFLOAT,    DXGI_FORMAT_R32G32B32_FLOAT,     GL_RGB32F,            FS_C(float,    3)     },
    { Format::RGBA8,           VK_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,      GL_RGBA8,             FS_C(uint8_t,  4)     },
    { Format::RGBA16,          VK_FORMAT_R16G16B16A16_UNORM,  DXGI_FORMAT_R16G16B16A16_UNORM,  GL_RGBA16,            FS_C(uint16_t, 4)     },
    { Format::RGBA16F,         VK_FORMAT_R16G16B16A16_SFLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,  GL_RGBA16F,           FS_C(bfloat,   4)     },
    { Format::RGBA32,          VK_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_R32G32B32A32_SINT,   GL_INT,               FS_C(int,      4)     },
    { Format::RGBA32F,         VK_FORMAT_R32G32B32A32_SFLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,  GL_RGBA32F,           FS_C(float,    4)     },
    { Format::BGRA8,           VK_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_B8G8R8A8_UNORM,      GL_RGBA8,             FS_C(uint8_t,  4)     },
    { Format::SRGB,            VK_FORMAT_R8G8B8A8_SRGB,       DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, GL_SRGB8,             FS_C(uint8_t,  4)     },
    { Format::Depth32F,        VK_FORMAT_D32_SFLOAT,          DXGI_FORMAT_D32_FLOAT,           GL_DEPTH32F_STENCIL8, FS_C(float,    1)     },
    { Format::Depth24Stencil8, VK_FORMAT_D24_UNORM_S8_UINT,   DXGI_FORMAT_D24_UNORM_S8_UINT,   GL_DEPTH24_STENCIL8,  FS_C(uint32_t, 1)     },
    { Format::YUV420P,         VK_FORMAT_UNDEFINED,           DXGI_FORMAT_UNKNOWN,             GL_INVALID_ENUM,      FS_C(uint8_t,  3)     },
    { Format::YUV422P,         VK_FORMAT_UNDEFINED,           DXGI_FORMAT_UNKNOWN,             GL_INVALID_ENUM,      FS_C(uint8_t,  3)     },
    { Format::YUV444P,         VK_FORMAT_UNDEFINED,           DXGI_FORMAT_UNKNOWN,             GL_INVALID_ENUM,      FS_C(uint8_t,  3)     },
};

}

inline size_t Format::ComponentCount() const
{
    return StaticSpace::FromatElementTable[v].ComponentCount;
}

inline Format::operator VkFormat() const
{
    return StaticSpace::FromatElementTable[v];
}

inline Format::operator DXGI_FORMAT() const
{
    return StaticSpace::FromatElementTable[v];
}

inline Format::operator GLenum() const
{
    return StaticSpace::FromatElementTable[v];
}

inline size_t Format::Size() const
{
    return StaticSpace::FromatElementTable[v].Size;
}

}
