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
#ifdef __APPLE__
#include <Metal/MTLPixelFormat.hpp>
#define MTLF(x) x
#else
#define MTLF(x) 0
namespace MTL
{
    using PixelFormat = int;
}
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
        R8_UNORM,
        R8_SNORM,
        R8_UINT,
        R8_SINT,
        R8_SRGB,
        R8G8_UNORM,
        R8G8_SNORM,
        R8G8_UINT,
        R8G8_SINT,
        R8G8_SRGB,
        R8G8B8_UNORM,
        R8G8B8_SNORM,
        R8G8B8_UINT,
        R8G8B8_SINT,
        R8G8B8_SRGB,
        B8G8R8_UNORM,
        B8G8R8_SNORM,
        B8G8R8_UINT,
        B8G8R8_SINT,
        B8G8R8_SRGB,
        R8G8B8A8_UNORM,
        R8G8B8A8_SNORM,
        R8G8B8A8_UINT,
        R8G8B8A8_SINT,
        R8G8B8A8_SRGB,
        B8G8R8A8_UNORM,
        B8G8R8A8_SNORM,
        B8G8R8A8_UINT,
        B8G8R8A8_SINT,
        B8G8R8A8_SRGB,
        R16_UNORM,
        R16_SNORM,
        R16_UINT,
        R16_SINT,
        R16_SFLOAT,
        R16G16_UNORM,
        R16G16_SNORM,
        R16G16_UINT,
        R16G16_SINT,
        R16G16_SFLOAT,
        R16G16B16_UNORM,
        R16G16B16_SNORM,
        R16G16B16_UINT,
        R16G16B16_SINT,
        R16G16B16_SFLOAT,
        R16G16B16A16_UNORM,
        R16G16B16A16_SNORM,
        R16G16B16A16_UINT,
        R16G16B16A16_SINT,
        R16G16B16A16_SFLOAT,
        R32_UINT,
        R32_SINT,
        R32_SFLOAT,
        R32G32_UINT,
        R32G32_SINT,
        R32G32_SFLOAT,
        R32G32B32_UINT,
        R32G32B32_SINT,
        R32G32B32_SFLOAT,
        R32G32B32A32_UINT,
        R32G32B32A32_SINT,
        R32G32B32A32_SFLOAT,
        Depth32F,
        Depth24Stencil8,
        BayerLayerRGGB,

        R8        = R8_UNORM,
        RG8       = R8G8_UNORM,
        RGBA8     = R8G8B8A8_UNORM,
        BGRA8     = B8G8R8A8_UNORM,
        R16       = R16_UNORM,
        RG16      = R16G16_UNORM,
        RGBA16    = R16G16B16A16_UNORM,
        INT       = R32_SINT,
		UINT16    = R16_UINT,
		UINT32    = R32_UINT,
        IVECTOR2  = R32G32_SINT,
        IVECTOR3  = R32G32B32_SINT,
        IVECTOR4  = R32G32B32A32_SINT,
        FLOAT     = R32_SFLOAT,
        VECTOR2   = R32G32_SFLOAT,
        VECTOR3   = R32G32B32_SFLOAT,
        VECTOR4   = R32G32B32A32_SFLOAT,

        YUV420P   = BITS(YUV,              BayerLayerRGGB + 1),
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

    static constexpr ValueType HightBitDepth = ValueType(Format::_10Bits | Format::_12Bits | Format::_16Bits);

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

#ifdef __APPLE__
    operator MTL::PixelFormat() const;
#endif

private:
    ValueType v;
};

}
