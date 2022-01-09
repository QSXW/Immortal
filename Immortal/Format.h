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
enum class Format
{
    R8G8B8A8_UNORM = 0,
    R8G8B8A8_SRGB = 1,
    UNDEFINED     = -1,
    INT           = 0,
    IVECTOR2,
    IVECTOR3,
    IVECTOR4,
    FLOAT,
    VECTOR2,
    VECTOR3,
    VECTOR4,
    COLOUR = VECTOR4,
    MATRIX4,
    RGBA8,
    RGBA16,
    BGRA8,
    RGBA16F,
    RGBA32F,
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
    RGB32,
    RGB32F,
    SRGB,
    Depth32F,
    Depth24Stencil8,
    RGB8,
    Depth = Depth32F,
    None
};

namespace Map
{

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

using bfloat = uint16_t;
#define FS_C(T, CC) (sizeof(T) * (CC)), CC

static inline BaseFormatElement BaseFormatMapper[] = {
    { VK_FORMAT_R32_SINT,            DXGI_FORMAT_R32_SINT,            GL_INT,               FS_C(int,      1)     },
    { VK_FORMAT_R32G32_SINT,         DXGI_FORMAT_R32G32_SINT,         GL_INT,               FS_C(int,      2)     },
    { VK_FORMAT_R32G32B32_SINT,      DXGI_FORMAT_R32G32B32_SINT,      GL_INT,               FS_C(int,      3)     },
    { VK_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_R32G32B32A32_SINT,   GL_INT,               FS_C(int,      4)     },
    { VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,           GL_FLOAT,             FS_C(float,    1)     },
    { VK_FORMAT_R32G32_SFLOAT,       DXGI_FORMAT_R32G32_FLOAT,        GL_FLOAT,             FS_C(float,    2)     },
    { VK_FORMAT_R32G32B32_SFLOAT,    DXGI_FORMAT_R32G32B32_FLOAT,     GL_FLOAT,             FS_C(float,    3)     },
    { VK_FORMAT_R32G32B32A32_SFLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,  GL_FLOAT,             FS_C(float,    4)     },
    { VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,           GL_FLOAT,             FS_C(float,    4 * 4) },
    { VK_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,      GL_RGBA8,             FS_C(uint8_t,  4)     },
    { VK_FORMAT_R16G16B16A16_UNORM,  DXGI_FORMAT_R16G16B16A16_UNORM,  GL_RGBA16,            FS_C(uint16_t, 4)     },
    { VK_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_B8G8R8A8_UNORM,      GL_RGBA8,             FS_C(uint8_t,  4)     },
    { VK_FORMAT_R16G16B16A16_SFLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,  GL_RGBA16F,           FS_C(bfloat,   4)     },
    { VK_FORMAT_R32G32B32A32_SFLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,  GL_RGBA32F,           FS_C(float,    4)     },
    { VK_FORMAT_R8_UNORM,            DXGI_FORMAT_R8_UNORM,            GL_R8,                FS_C(uint8_t,  1)     },
    { VK_FORMAT_R16_UNORM,           DXGI_FORMAT_R16_UNORM,           GL_R16,               FS_C(uint16_t, 1)     },
    { VK_FORMAT_R16_SFLOAT,          DXGI_FORMAT_R16_FLOAT,           GL_R32F,              FS_C(bfloat,   1)     },
    { VK_FORMAT_R32_UINT,            DXGI_FORMAT_R32_UINT,            GL_R32UI ,            FS_C(uint32_t, 1)     },
    { VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,           GL_R32F,              FS_C(float,    1)     },
    { VK_FORMAT_R8G8_UNORM,          DXGI_FORMAT_R8G8_UNORM,          GL_RG8,               FS_C(uint8_t,  2)     },
    { VK_FORMAT_R16G16_UNORM,        DXGI_FORMAT_R16G16_UNORM,        GL_RG16,              FS_C(uint16_t, 2)     },
    { VK_FORMAT_R16G16_SFLOAT,       DXGI_FORMAT_R16G16_FLOAT,        GL_RG32F,             FS_C(bfloat,   2)     },
    { VK_FORMAT_R32G32_UINT,         DXGI_FORMAT_R32G32_UINT,         GL_RG32UI ,           FS_C(uint32_t, 2)     },
    { VK_FORMAT_R32G32_SFLOAT,       DXGI_FORMAT_R32G32_FLOAT,        GL_RG32F,             FS_C(float,    2)     },
    { VK_FORMAT_R32G32B32_UINT,      DXGI_FORMAT_R32G32B32_UINT,      GL_RGB32UI ,          FS_C(uint32_t, 3)     },
    { VK_FORMAT_R32G32B32_SFLOAT,    DXGI_FORMAT_R32G32B32_FLOAT,     GL_RGB32F,            FS_C(float,    3)     },
    { VK_FORMAT_R8G8B8A8_SRGB,       DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, GL_SRGB8,             FS_C(uint8_t,  4)     },
    { VK_FORMAT_D32_SFLOAT,          DXGI_FORMAT_D32_FLOAT,           GL_DEPTH32F_STENCIL8, FS_C(float,    1)     }
};

template <class T>
static inline T BaseFormat(Format format)
{
    if constexpr (IsPrimitiveOf<GLenum, T>())
    {
        return BaseFormatMapper[ncast<int>(format)].OpenGL;
    }
    if constexpr (IsPrimitiveOf<VkFormat, T>())
    {
        return BaseFormatMapper[ncast<int>(format)].Vulkan;
    }
    if constexpr (IsPrimitiveOf<DXGI_FORMAT, T>())
    {
        return BaseFormatMapper[ncast<int>(format)].DXGI;
    }
    return ncast<T>(format);
}

static inline auto FormatComponentCount(Format format)
{
    return BaseFormatMapper[ncast<int>(format)].componentCount;
}

static inline auto FormatSize(Format format)
{
    return BaseFormatMapper[ncast<int>(format)].size;
}

}
#undef FS_C

};
