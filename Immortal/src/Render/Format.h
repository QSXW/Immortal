#pragma once

#include "ImmortalCore.h"
#include "Platform/OpenGL/Common.h"
#include "Platform/Vulkan/Common.h"
#include "Platform/D3D12/Common.h"

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
    BGRA8,
    RGBA16F,
    RGBA32F,
    R8,
    R16,
    R32,
    R32F,
    RG32F,
    RG16F,
    SRGB,
    RGB16F,
    RGB32F,
    Depth32F,
    Depth24Stencil8,
    Depth = Depth24Stencil8,
    RGB8,
    None
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

using bfloat = uint16_t;
#define FS_C(T, CC) (sizeof(T) * (CC)), CC

static inline BaseFormatElement BaseFormatMapper[] = {
    { VK_FORMAT_R32_SINT,            DXGI_FORMAT_R32_SINT,           GL_INT,     FS_C(int,     1)     },
    { VK_FORMAT_R32G32_SINT,         DXGI_FORMAT_R32G32_SINT,        GL_INT,     FS_C(int,     2)     },
    { VK_FORMAT_R32G32B32_SINT,      DXGI_FORMAT_R32G32B32_SINT,     GL_INT,     FS_C(int,     3)     },
    { VK_FORMAT_R32G32B32A32_SINT,   DXGI_FORMAT_R32G32B32A32_SINT,  GL_INT,     FS_C(int,     4)     },
    { VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,          GL_FLOAT,   FS_C(float,   1)     },
    { VK_FORMAT_R32G32_SFLOAT,       DXGI_FORMAT_R32G32_FLOAT,       GL_FLOAT,   FS_C(float,   1)     },
    { VK_FORMAT_R32G32B32_SFLOAT,    DXGI_FORMAT_R32G32B32_FLOAT,    GL_FLOAT,   FS_C(float,   2)     },
    { VK_FORMAT_R32G32B32A32_SFLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, GL_FLOAT,   FS_C(float,   1)     },
    { VK_FORMAT_R32_SFLOAT,          DXGI_FORMAT_R32_FLOAT,          GL_FLOAT,   FS_C(float,   4 * 4) },
    { VK_FORMAT_R8G8B8A8_UNORM,      DXGI_FORMAT_R8G8B8A8_UNORM,     GL_RGBA8,   FS_C(uint8_t, 4)     },
    { VK_FORMAT_B8G8R8A8_UNORM,      DXGI_FORMAT_B8G8R8A8_UNORM,     GL_RGBA8,   FS_C(uint8_t, 4)     },
    { VK_FORMAT_R16G16B16A16_SFLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, GL_RGBA16F, FS_C(bfloat,  4)     },
    { VK_FORMAT_R32G32B32A32_SFLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, GL_RGBA32F, FS_C(float,   4)     }
};

#undef FS

};
