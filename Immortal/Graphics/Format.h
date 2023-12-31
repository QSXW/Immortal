#pragma once

#include "Core.h"
#include "OpenGL/GLFormat.h"
#include <vulkan/vulkan_core.h>

#ifdef _WIN32
#include <dxgiformat.h>
#define DXF(x) x
#else
#define DXF(x) 0
using DXGI_FORMAT = int;
#endif

namespace Immortal
{

constexpr uint32_t BITS(uint32_t a, uint32_t b)
{
    return a | b;
}

constexpr uint32_t BITS(uint32_t a, uint32_t b, uint32_t c)
{
    return a | b | c;
}

constexpr uint32_t BITS(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    return a | b | c | d;
}

/**
 * @brief: FLOAT here represents 32-bits floating points
 *         INT represents 32-bits signed integers
 */
struct Format
{
public:
    enum ValueType : uint32_t
    {
        YUV     = BIT(16),
        NV      = BIT(17),
        _10Bits = BIT(18),
		_12Bits = BIT(19),
        _16Bits = BIT(20),

        None = 0,
        INT,
		UINT16,
		UINT32,
        IVECTOR2,
        IVECTOR3,
        IVECTOR4,
        FLOAT,
        VECTOR2,
        VECTOR3,
        VECTOR4,
        COLOUR = VECTOR4,
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
        YUV420P   = BITS(YUV,              Depth24Stencil8 + 1),
        YUV422P   = BITS(YUV,              YUV420P + 1 ),
        YUV444P   = BITS(YUV,              YUV420P + 2 ),
        YUV420P10 = BITS(YUV,     _10Bits, YUV420P + 3 ),
        YUV422P10 = BITS(YUV,     _10Bits, YUV420P + 4 ),
        YUV444P10 = BITS(YUV,     _10Bits, YUV420P + 5 ),
		YUV420P12 = BITS(YUV,     _12Bits, YUV420P + 6 ),
		YUV422P12 = BITS(YUV,     _12Bits, YUV420P + 7 ),
		YUV444P12 = BITS(YUV,     _12Bits, YUV420P + 8 ),
        YUV420P16 = BITS(YUV,     _16Bits, YUV420P + 9 ),
		YUV422P16 = BITS(YUV,     _16Bits, YUV420P + 10),
		YUV444P16 = BITS(YUV,     _16Bits, YUV420P + 11),
        NV12      = BITS(YUV, NV,          YUV420P + 12),
        P010LE    = BITS(YUV, NV, _10Bits, YUV420P + 13),
        P016LE    = BITS(YUV,     _16Bits, YUV420P + 14),
        Y210      = BITS(YUV,     _10Bits, YUV420P + 15),
		Y216      = BITS(YUV,     _16Bits, YUV420P + 16)
    };

    Format() :
        v{ None }
    {

    }

    Format(ValueType v) :
        v{ v }
    {

    }

    Format(VkFormat format);

    bool IsType(const Format other) const
    {
        return (other & v) != Format::None;
    }

    bool IsDepth() const
    {
        return v == Depth32F || v == Depth24Stencil8;
    }

    operator ValueType() const
    {
        return v;
    }

    Format operator&(const ValueType other) const
    {
        return Format(ValueType(other & v));
    }

    Format operator &(const Format other) const
    {
        return Format(ValueType(other.v & v));
    }

    int ComponentCount() const;

    int GetComponent() const;

    int ElementSize() const;

    int BytesPerPixel() const;

    size_t Size() const;

    size_t GetTexelSize() const;

    operator VkFormat() const;

    operator DXGI_FORMAT() const;

    operator GL_FORMAT() const;

private:
    ValueType v;
};

namespace StaticSpace
{

struct FormatElement
{
    FormatElement(Format format, VkFormat vkFormat, DXGI_FORMAT dxgiFormat, GL_FORMAT glFormat, int size, int count) :
        vk{ vkFormat },
#ifdef _WIN32
        dxgi{dxgiFormat},
#endif
        ogl{ glFormat},
        ElementSize{ size },
        ComponentCount{ count }
    {

    }

    VkFormat vk;
    operator VkFormat() const
    {
        return vk;
    }

#ifdef _WIN32
    DXGI_FORMAT dxgi;
    operator DXGI_FORMAT() const
    {
        return dxgi;
    }
#endif

    GL_FORMAT ogl;
    operator GL_FORMAT() const
    {
        return ogl;
    }

    int         ElementSize;
    int         ComponentCount;
};

using bfloat = uint16_t;
#define FS_C(T, CC) (sizeof(T)), CC

static inline FormatElement FormatElementTable[] = {
    { Format::None,            VK_FORMAT_UNDEFINED,                     DXF(DXGI_FORMAT_UNKNOWN            ),  GL_FORMAT_INVALID_ENUM,      FS_C(int,      0)     },
    { Format::INT,             VK_FORMAT_R32_SINT,                      DXF(DXGI_FORMAT_R32_SINT           ),  GL_FORMAT_INT,               FS_C(int,      1)     },
    { Format::UINT16,          VK_FORMAT_R16_UINT,                      DXF(DXGI_FORMAT_R16_UINT           ),  GL_FORMAT_UNSIGNED_SHORT,    FS_C(uint16_t, 1)     },
    { Format::UINT32,          VK_FORMAT_R32_UINT,                      DXF(DXGI_FORMAT_R32_UINT           ),  GL_FORMAT_UNSIGNED_INT,      FS_C(uint32_t, 1)     },
    { Format::IVECTOR2,        VK_FORMAT_R32G32_SINT,                   DXF(DXGI_FORMAT_R32G32_SINT        ),  GL_FORMAT_RG32I,             FS_C(int,      2)     },
    { Format::IVECTOR3,        VK_FORMAT_R32G32B32_SINT,                DXF(DXGI_FORMAT_R32G32B32_SINT     ),  GL_FORMAT_RGB32I,            FS_C(int,      3)     },
    { Format::IVECTOR4,        VK_FORMAT_R32G32B32A32_SINT,             DXF(DXGI_FORMAT_R32G32B32A32_SINT  ),  GL_FORMAT_RGBA32I,           FS_C(int,      4)     },
    { Format::FLOAT,           VK_FORMAT_R32_SFLOAT,                    DXF(DXGI_FORMAT_R32_FLOAT          ),  GL_FORMAT_FLOAT,             FS_C(float,    1)     },
    { Format::VECTOR2,         VK_FORMAT_R32G32_SFLOAT,                 DXF(DXGI_FORMAT_R32G32_FLOAT       ),  GL_FORMAT_RG32F,             FS_C(float,    2)     },
    { Format::VECTOR3,         VK_FORMAT_R32G32B32_SFLOAT,              DXF(DXGI_FORMAT_R32G32B32_FLOAT    ),  GL_FORMAT_RGB32F,            FS_C(float,    3)     },
    { Format::VECTOR4,         VK_FORMAT_R32G32B32A32_SFLOAT,           DXF(DXGI_FORMAT_R32G32B32A32_FLOAT ),  GL_FORMAT_RGBA32F,           FS_C(float,    4)     },
    { Format::R8,              VK_FORMAT_R8_UNORM,                      DXF(DXGI_FORMAT_R8_UNORM           ),  GL_FORMAT_R8,                FS_C(uint8_t,  1)     },
    { Format::R16,             VK_FORMAT_R16_UNORM,                     DXF(DXGI_FORMAT_R16_UNORM          ),  GL_FORMAT_R16,               FS_C(uint16_t, 1)     },
    { Format::R16F,            VK_FORMAT_R16_SFLOAT,                    DXF(DXGI_FORMAT_R16_FLOAT          ),  GL_FORMAT_R32F,              FS_C(bfloat,   1)     },
    { Format::R32,             VK_FORMAT_R32_SINT,                      DXF(DXGI_FORMAT_R32_SINT           ),  GL_FORMAT_R32I ,             FS_C(uint32_t, 1)     },
    { Format::R32F,            VK_FORMAT_R32_SFLOAT,                    DXF(DXGI_FORMAT_R32_FLOAT          ),  GL_FORMAT_R32F,              FS_C(float,    1)     },
    { Format::RG8,             VK_FORMAT_R8G8_UNORM,                    DXF(DXGI_FORMAT_R8G8_UNORM         ),  GL_FORMAT_RG8,               FS_C(uint8_t,  2)     },
    { Format::RG16,            VK_FORMAT_R16G16_UNORM,                  DXF(DXGI_FORMAT_R16G16_UNORM       ),  GL_FORMAT_RG16,              FS_C(uint16_t, 2)     },
    { Format::RG16F,           VK_FORMAT_R16G16_SFLOAT,                 DXF(DXGI_FORMAT_R16G16_FLOAT       ),  GL_FORMAT_RG32F,             FS_C(bfloat,   2)     },
    { Format::RG32,            VK_FORMAT_R32G32_SINT,                   DXF(DXGI_FORMAT_R32G32_SINT        ),  GL_FORMAT_RG32I ,            FS_C(uint32_t, 2)     },
    { Format::RG32F,           VK_FORMAT_R32G32_SFLOAT,                 DXF(DXGI_FORMAT_R32G32_FLOAT       ),  GL_FORMAT_RG32F,             FS_C(float,    2)     },
    { Format::RGB8,            VK_FORMAT_R8G8B8_UNORM,                  DXF(DXGI_FORMAT_UNKNOWN            ),  GL_FORMAT_RGB8 ,             FS_C(uint8_t,  3)     },
    { Format::RGB32,           VK_FORMAT_R32G32B32_SINT,                DXF(DXGI_FORMAT_R32G32B32_SINT     ),  GL_FORMAT_RGB32I ,           FS_C(uint32_t, 3)     },
    { Format::RGB32F,          VK_FORMAT_R32G32B32_SFLOAT,              DXF(DXGI_FORMAT_R32G32B32_FLOAT    ),  GL_FORMAT_RGB32F,            FS_C(float,    3)     },
    { Format::RGBA8,           VK_FORMAT_R8G8B8A8_UNORM,                DXF(DXGI_FORMAT_R8G8B8A8_UNORM     ),  GL_FORMAT_RGBA8,             FS_C(uint8_t,  4)     },
    { Format::RGBA16,          VK_FORMAT_R16G16B16A16_UNORM,            DXF(DXGI_FORMAT_R16G16B16A16_UNORM ),  GL_FORMAT_RGBA16,            FS_C(uint16_t, 4)     },
    { Format::RGBA16F,         VK_FORMAT_R16G16B16A16_SFLOAT,           DXF(DXGI_FORMAT_R16G16B16A16_FLOAT ),  GL_FORMAT_RGBA16F,           FS_C(bfloat,   4)     },
    { Format::RGBA32,          VK_FORMAT_R32G32B32A32_SINT,             DXF(DXGI_FORMAT_R32G32B32A32_SINT  ),  GL_FORMAT_RGBA32I,           FS_C(int,      4)     },
    { Format::RGBA32F,         VK_FORMAT_R32G32B32A32_SFLOAT,           DXF(DXGI_FORMAT_R32G32B32A32_FLOAT ),  GL_FORMAT_RGBA32F,           FS_C(float,    4)     },
    { Format::BGRA8,           VK_FORMAT_B8G8R8A8_UNORM,                DXF(DXGI_FORMAT_B8G8R8A8_UNORM     ),  GL_FORMAT_RGBA8,             FS_C(uint8_t,  4)     },
    { Format::SRGB,            VK_FORMAT_R8G8B8A8_SRGB,                 DXF(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),  GL_FORMAT_SRGB8,             FS_C(uint8_t,  4)     },
    { Format::Depth32F,        VK_FORMAT_D32_SFLOAT,                    DXF(DXGI_FORMAT_D32_FLOAT          ),  GL_FORMAT_DEPTH32F_STENCIL8, FS_C(float,    1)     },
    { Format::Depth24Stencil8, VK_FORMAT_D24_UNORM_S8_UINT,             DXF(DXGI_FORMAT_D24_UNORM_S8_UINT  ),  GL_FORMAT_DEPTH24_STENCIL8,  FS_C(uint32_t, 1)     },
    { Format::YUV420P,         VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,     DXF(DXGI_FORMAT_UNKNOWN            ),  GL_FORMAT_INVALID_ENUM,      FS_C(uint8_t,  3)     },
    { Format::YUV422P,         VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,     DXF(DXGI_FORMAT_YUY2               ), GL_FORMAT_INVALID_ENUM,       FS_C(uint8_t,  3)     },
    { Format::YUV444P,         VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,     DXF(DXGI_FORMAT_AYUV               ), GL_FORMAT_INVALID_ENUM,       FS_C(uint8_t,  3)     },
    { Format::YUV420P10,       VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,  DXF(DXGI_FORMAT_UNKNOWN            ),  GL_FORMAT_INVALID_ENUM,      FS_C(uint16_t, 3)     },
    { Format::YUV422P10,       VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,  DXF(DXGI_FORMAT_Y210               ),  GL_FORMAT_INVALID_ENUM,      FS_C(uint16_t, 3)     },
    { Format::YUV444P10,       VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,  DXF(DXGI_FORMAT_Y410               ),  GL_FORMAT_INVALID_ENUM,      FS_C(uint16_t, 3)     },
    { Format::NV12,            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,      DXF(DXGI_FORMAT_NV12               ),  GL_FORMAT_INVALID_ENUM,      FS_C(uint8_t,  3)     },
    { Format::P010LE,          VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,   DXF(DXGI_FORMAT_P010               ),  GL_FORMAT_INVALID_ENUM,      FS_C(uint16_t, 3)     },
    { Format::P010LE,          VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,   DXF(DXGI_FORMAT_P016               ),  GL_FORMAT_INVALID_ENUM,      FS_C(uint16_t, 3)     },
};

static const FormatElement &GetFormatElement(const Format &format)
{
    size_t index = Format::ValueType(format) & 0xffff;
    return FormatElementTable[index];
}

}

inline int Format::ComponentCount() const
{
    return StaticSpace::GetFormatElement(*this).ComponentCount;
}

inline int Format::GetComponent() const
{
	return StaticSpace::GetFormatElement(*this).ComponentCount;
}

inline Format::operator VkFormat() const
{
    return StaticSpace::GetFormatElement(*this);
}

inline Format::operator DXGI_FORMAT() const
{
    return StaticSpace::GetFormatElement(*this);
}

inline Format::operator GL_FORMAT() const
{
    return StaticSpace::GetFormatElement(*this);
}

inline int Format::ElementSize() const
{
    return StaticSpace::GetFormatElement(*this).ElementSize;
}

inline int Format::BytesPerPixel() const
{
    return ElementSize();
}

inline size_t Format::Size() const
{
    return ElementSize() * ComponentCount();
}

inline size_t Format::GetTexelSize() const
{
	return ElementSize() * ComponentCount();
}

inline Format::Format(VkFormat format) :
    v{}
{
    for (size_t i = 0; i < SL_ARRAY_LENGTH(StaticSpace::FormatElementTable); i++)
    {
		auto &table = StaticSpace::FormatElementTable[i];
        if (table.vk == format)
        {
			v = ValueType(i);
        }
    }
}

}
