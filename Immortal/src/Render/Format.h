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
    RedInterger,
    RGB,
    RGB8,
    RGBA,
    RGBA8,
    RGBA16F,
    RGBA32F,
    RG32F,
    RG16F,
    BGRA8,
    SRGB,
    RGB16F,
    RGB32F,
    Depth32F,
    Depth24Stencil8,
    Depth = Depth24Stencil8,
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

static inline BaseFormatElement BaseFormatMapper[] = {
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

}
