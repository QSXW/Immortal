#include "Format.h"

namespace Immortal
{

namespace StaticSpace
{

struct FormatElement
{
    FormatElement(Format format, VkFormat vkFormat, DXGI_FORMAT dxgiFormat, MTL::PixelFormat mtlFormat, GL_FORMAT glFormat, int size, int count) :
        vk{ vkFormat },
#ifdef _WIN32
        dxgi{dxgiFormat},
#endif
#ifdef __APPLE__
        mtl{ mtlFormat },
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

#ifdef __APPLE__
    MTL::PixelFormat mtl;
    operator MTL::PixelFormat() const
    {
        return mtl;
    }
#endif

    int         ElementSize;
    int         ComponentCount;
};

using bfloat = uint16_t;
#define FS_C(T, CC) (sizeof(T)), CC

static inline FormatElement FormatElementTable[] = {
    { Format::None,                VK_FORMAT_UNDEFINED,                     DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  1)  },
    { Format::R8_UNORM,            VK_FORMAT_R8_UNORM,                      DXF(DXGI_FORMAT_R8_UNORM           ), MTLF(MTL::PixelFormatR8Unorm              ), GL_FORMAT_R8,                  FS_C(uint8_t,  1)  },
    { Format::R8_SNORM,            VK_FORMAT_R8_SNORM,                      DXF(DXGI_FORMAT_R8_SNORM           ), MTLF(MTL::PixelFormatR8Snorm              ), GL_FORMAT_R8_SNORM,            FS_C(int8_t,   1)  },
    { Format::R8_UINT,             VK_FORMAT_R8_UINT,                       DXF(DXGI_FORMAT_R8_UINT            ), MTLF(MTL::PixelFormatR8Uint               ), GL_FORMAT_R8UI,                FS_C(uint8_t,  1)  },
    { Format::R8_SINT,             VK_FORMAT_R8_SINT,                       DXF(DXGI_FORMAT_R8_SINT            ), MTLF(MTL::PixelFormatR8Sint               ), GL_FORMAT_R8I,                 FS_C(int8_t,   1)  },
    { Format::R8_SRGB,             VK_FORMAT_R8_SRGB,                       DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatR8Unorm_sRGB         ), GL_FORMAT_INVALID,             FS_C(uint8_t,  1)  },
    { Format::R8G8_UNORM,          VK_FORMAT_R8G8_UNORM,                    DXF(DXGI_FORMAT_R8G8_UNORM         ), MTLF(MTL::PixelFormatRG8Unorm             ), GL_FORMAT_RG8,                 FS_C(uint8_t,  2)  },
    { Format::R8G8_SNORM,          VK_FORMAT_R8G8_SNORM,                    DXF(DXGI_FORMAT_R8G8_SNORM         ), MTLF(MTL::PixelFormatRG8Snorm             ), GL_FORMAT_RG8_SNORM,           FS_C(int8_t,   2)  },
    { Format::R8G8_UINT,           VK_FORMAT_R8G8_UINT,                     DXF(DXGI_FORMAT_R8G8_UINT          ), MTLF(MTL::PixelFormatRG8Uint              ), GL_FORMAT_RG8UI,               FS_C(uint8_t,  2)  },
    { Format::R8G8_SINT,           VK_FORMAT_R8G8_SINT,                     DXF(DXGI_FORMAT_R8G8_SINT          ), MTLF(MTL::PixelFormatRG8Sint              ), GL_FORMAT_RG8I,                FS_C(int8_t,   2)  },
    { Format::R8G8_SRGB,           VK_FORMAT_R8G8_SRGB,                     DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatRG8Unorm_sRGB        ), GL_FORMAT_INVALID,             FS_C(uint8_t,  2)  },
    { Format::R8G8B8_UNORM,        VK_FORMAT_R8G8B8_UNORM,                  DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB8,                FS_C(uint8_t,  3)  },
    { Format::R8G8B8_SNORM,        VK_FORMAT_R8G8B8_SNORM,                  DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB8_SNORM,          FS_C(int8_t,   3)  },
    { Format::R8G8B8_UINT,         VK_FORMAT_R8G8B8_UINT,                   DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB8UI,              FS_C(uint8_t,  3)  },
    { Format::R8G8B8_SINT,         VK_FORMAT_R8G8B8_SINT,                   DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB8I,               FS_C(int8_t,   3)  },
    { Format::R8G8B8_SRGB,         VK_FORMAT_R8G8B8_SRGB,                   DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  3)  },
    { Format::B8G8R8_UNORM,        VK_FORMAT_B8G8R8_UNORM,                  DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  3)  },
    { Format::B8G8R8_SNORM,        VK_FORMAT_B8G8R8_SNORM,                  DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(int8_t,   3)  },
    { Format::B8G8R8_UINT,         VK_FORMAT_B8G8R8_UINT,                   DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  3)  },
    { Format::B8G8R8_SINT,         VK_FORMAT_B8G8R8_SINT,                   DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(int8_t,   3)  },
    { Format::B8G8R8_SRGB,         VK_FORMAT_B8G8R8_SRGB,                   DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  3)  },
    { Format::R8G8B8A8_UNORM,      VK_FORMAT_R8G8B8A8_UNORM,                DXF(DXGI_FORMAT_R8G8B8A8_UNORM     ), MTLF(MTL::PixelFormatRGBA8Unorm           ), GL_FORMAT_RGBA8,               FS_C(uint8_t,  4)  },
    { Format::R8G8B8A8_SNORM,      VK_FORMAT_R8G8B8A8_SNORM,                DXF(DXGI_FORMAT_R8G8B8A8_SNORM     ), MTLF(MTL::PixelFormatRGBA8Snorm           ), GL_FORMAT_RGBA8_SNORM,         FS_C(int8_t,   4)  },
    { Format::R8G8B8A8_UINT,       VK_FORMAT_R8G8B8A8_UINT,                 DXF(DXGI_FORMAT_R8G8B8A8_UINT      ), MTLF(MTL::PixelFormatRGBA8Uint            ), GL_FORMAT_RGBA8UI,             FS_C(uint8_t,  4)  },
    { Format::R8G8B8A8_SINT,       VK_FORMAT_R8G8B8A8_SINT,                 DXF(DXGI_FORMAT_R8G8B8A8_SINT      ), MTLF(MTL::PixelFormatRGBA8Sint            ), GL_FORMAT_RGBA8I,              FS_C(int8_t,   4)  },
    { Format::R8G8B8A8_SRGB,       VK_FORMAT_R8G8B8A8_SRGB,                 DXF(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB), MTLF(MTL::PixelFormatRGBA8Unorm_sRGB      ), GL_FORMAT_SRGB8,               FS_C(uint8_t,  4)  },
    { Format::B8G8R8A8_UNORM,      VK_FORMAT_B8G8R8A8_UNORM,                DXF(DXGI_FORMAT_B8G8R8A8_UNORM     ), MTLF(MTL::PixelFormatBGRA8Unorm           ), GL_FORMAT_INVALID,             FS_C(uint8_t,  4)  },
    { Format::B8G8R8A8_SNORM,      VK_FORMAT_B8G8R8A8_SNORM,                DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(int8_t,   4)  },
    { Format::B8G8R8A8_UINT,       VK_FORMAT_B8G8R8A8_UINT,                 DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  4)  },
    { Format::B8G8R8A8_SINT,       VK_FORMAT_B8G8R8A8_SINT,                 DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(int8_t,   4)  },
    { Format::B8G8R8A8_SRGB,       VK_FORMAT_B8G8R8A8_SRGB,                 DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatBGRA8Unorm_sRGB      ), GL_FORMAT_INVALID,             FS_C(uint8_t,  4)  },
    { Format::R16_UNORM,           VK_FORMAT_R16_UNORM,                     DXF(DXGI_FORMAT_R16_UNORM          ), MTLF(MTL::PixelFormatR16Unorm             ), GL_FORMAT_R16,                 FS_C(uint16_t, 1)  },
    { Format::R16_SNORM,           VK_FORMAT_R16_SNORM,                     DXF(DXGI_FORMAT_R16_SNORM          ), MTLF(MTL::PixelFormatR16Snorm             ), GL_FORMAT_R16_SNORM,           FS_C(int16_t,  1)  },
    { Format::R16_UINT,            VK_FORMAT_R16_UINT,                      DXF(DXGI_FORMAT_R16_UINT           ), MTLF(MTL::PixelFormatR16Uint              ), GL_FORMAT_R16UI,               FS_C(uint16_t, 1)  },
    { Format::R16_SINT,            VK_FORMAT_R16_SINT,                      DXF(DXGI_FORMAT_R16_SINT           ), MTLF(MTL::PixelFormatR16Sint              ), GL_FORMAT_R16I,                FS_C(int16_t,  1)  },
    { Format::R16_SFLOAT,          VK_FORMAT_R16_SFLOAT,                    DXF(DXGI_FORMAT_R16_FLOAT          ), MTLF(MTL::PixelFormatR16Float             ), GL_FORMAT_R16F,                FS_C(bfloat,   1)  },
    { Format::R16G16_UNORM,        VK_FORMAT_R16G16_UNORM,                  DXF(DXGI_FORMAT_R16G16_UNORM       ), MTLF(MTL::PixelFormatRG16Unorm            ), GL_FORMAT_RG16,                FS_C(uint16_t, 2)  },
    { Format::R16G16_SNORM,        VK_FORMAT_R16G16_SNORM,                  DXF(DXGI_FORMAT_R16G16_SNORM       ), MTLF(MTL::PixelFormatRG16Snorm            ), GL_FORMAT_RG16_SNORM,          FS_C(int16_t,  2)  },
    { Format::R16G16_UINT,         VK_FORMAT_R16G16_UINT,                   DXF(DXGI_FORMAT_R16G16_UINT        ), MTLF(MTL::PixelFormatRG16Uint             ), GL_FORMAT_RG16UI,              FS_C(uint16_t, 2)  },
    { Format::R16G16_SINT,         VK_FORMAT_R16G16_SINT,                   DXF(DXGI_FORMAT_R16G16_SINT        ), MTLF(MTL::PixelFormatRG16Sint             ), GL_FORMAT_RG16I,               FS_C(int16_t,  2)  },
    { Format::R16G16_SFLOAT,       VK_FORMAT_R16G16_SFLOAT,                 DXF(DXGI_FORMAT_R16G16_FLOAT       ), MTLF(MTL::PixelFormatRG16Float            ), GL_FORMAT_RG16F,               FS_C(bfloat,   2)  },
    { Format::R16G16B16_UNORM,     VK_FORMAT_R16G16B16_UNORM,               DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB16,               FS_C(uint16_t, 3)  },
    { Format::R16G16B16_SNORM,     VK_FORMAT_R16G16B16_SNORM,               DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB16_SNORM,         FS_C(int16_t,  3)  },
    { Format::R16G16B16_UINT,      VK_FORMAT_R16G16B16_UINT,                DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB16UI,             FS_C(uint16_t, 3)  },
    { Format::R16G16B16_SINT,      VK_FORMAT_R16G16B16_SINT,                DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB16I,              FS_C(int16_t,  3)  },
    { Format::R16G16B16_SFLOAT,    VK_FORMAT_R16G16B16_SFLOAT,              DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB16F,              FS_C(bfloat,   3)  },
    { Format::R16G16B16A16_UNORM,  VK_FORMAT_R16G16B16A16_UNORM,            DXF(DXGI_FORMAT_R16G16B16A16_UNORM ), MTLF(MTL::PixelFormatRGBA16Unorm          ), GL_FORMAT_RGBA16,              FS_C(uint16_t, 4)  },
    { Format::R16G16B16A16_SNORM,  VK_FORMAT_R16G16B16A16_SNORM,            DXF(DXGI_FORMAT_R16G16B16A16_SNORM ), MTLF(MTL::PixelFormatRGBA16Snorm          ), GL_FORMAT_RGBA16_SNORM,        FS_C(int16_t,  4)  },
    { Format::R16G16B16A16_UINT,   VK_FORMAT_R16G16B16A16_UINT,             DXF(DXGI_FORMAT_R16G16B16A16_UINT  ), MTLF(MTL::PixelFormatRGBA16Uint           ), GL_FORMAT_RGBA16UI,            FS_C(uint16_t, 4)  },
    { Format::R16G16B16A16_SINT,   VK_FORMAT_R16G16B16A16_SINT,             DXF(DXGI_FORMAT_R16G16B16A16_SINT  ), MTLF(MTL::PixelFormatRGBA16Sint           ), GL_FORMAT_RGBA16I,             FS_C(int16_t,  4)  },
    { Format::R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT,           DXF(DXGI_FORMAT_R16G16B16A16_FLOAT ), MTLF(MTL::PixelFormatRGBA16Float          ), GL_FORMAT_RGBA16F,             FS_C(bfloat,   4)  },
    { Format::R32_UINT,            VK_FORMAT_R32_UINT,                      DXF(DXGI_FORMAT_R32_UINT           ), MTLF(MTL::PixelFormatR32Uint              ), GL_FORMAT_R32UI,               FS_C(uint32_t, 1)  },
    { Format::R32_SINT,            VK_FORMAT_R32_SINT,                      DXF(DXGI_FORMAT_R32_SINT           ), MTLF(MTL::PixelFormatR32Sint              ), GL_FORMAT_R32I,                FS_C(int32_t,  1)  },
    { Format::R32_SFLOAT,          VK_FORMAT_R32_SFLOAT,                    DXF(DXGI_FORMAT_R32_FLOAT          ), MTLF(MTL::PixelFormatR32Float             ), GL_FORMAT_R32F,                FS_C(float,    1)  },
    { Format::R32G32_UINT,         VK_FORMAT_R32G32_UINT,                   DXF(DXGI_FORMAT_R32G32_UINT        ), MTLF(MTL::PixelFormatRG32Uint             ), GL_FORMAT_RG32UI,              FS_C(uint32_t, 2)  },
    { Format::R32G32_SINT,         VK_FORMAT_R32G32_SINT,                   DXF(DXGI_FORMAT_R32G32_SINT        ), MTLF(MTL::PixelFormatRG32Sint             ), GL_FORMAT_RG32I,               FS_C(int32_t,  2)  },
    { Format::R32G32_SFLOAT,       VK_FORMAT_R32G32_SFLOAT,                 DXF(DXGI_FORMAT_R32G32_FLOAT       ), MTLF(MTL::PixelFormatRG32Float            ), GL_FORMAT_RG32F,               FS_C(float,    2)  },
    { Format::R32G32B32_UINT,      VK_FORMAT_R32G32B32_UINT,                DXF(DXGI_FORMAT_R32G32B32_UINT     ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB32UI,             FS_C(uint32_t, 3)  },
    { Format::R32G32B32_SINT,      VK_FORMAT_R32G32B32_SINT,                DXF(DXGI_FORMAT_R32G32B32_SINT     ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB32I,              FS_C(int32_t,  3)  },
    { Format::R32G32B32_SFLOAT,    VK_FORMAT_R32G32B32_SFLOAT,              DXF(DXGI_FORMAT_R32G32B32_FLOAT    ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_RGB32F,              FS_C(float,    3)  },
    { Format::R32G32B32A32_UINT,   VK_FORMAT_R32G32B32A32_UINT,             DXF(DXGI_FORMAT_R32G32B32A32_UINT  ), MTLF(MTL::PixelFormatRGBA32Uint           ), GL_FORMAT_RGBA32UI,            FS_C(uint32_t, 4)  },
    { Format::R32G32B32A32_SINT,   VK_FORMAT_R32G32B32A32_SINT,             DXF(DXGI_FORMAT_R32G32B32A32_SINT  ), MTLF(MTL::PixelFormatRGBA32Sint           ), GL_FORMAT_RGBA32I,             FS_C(int32_t,  4)  },
    { Format::R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,           DXF(DXGI_FORMAT_R32G32B32A32_FLOAT ), MTLF(MTL::PixelFormatRGBA32Float          ), GL_FORMAT_RGBA32F,             FS_C(float,    4)  },
    { Format::Depth32F,            VK_FORMAT_D32_SFLOAT,                    DXF(DXGI_FORMAT_D32_FLOAT          ), MTLF(MTL::PixelFormatDepth32Float         ), GL_FORMAT_DEPTH32F_STENCIL8,   FS_C(float,    1)  },
    { Format::Depth24Stencil8,     VK_FORMAT_D24_UNORM_S8_UINT,             DXF(DXGI_FORMAT_D24_UNORM_S8_UINT  ), MTLF(MTL::PixelFormatDepth24Unorm_Stencil8), GL_FORMAT_DEPTH24_STENCIL8,    FS_C(uint32_t, 1)  },
    { Format::BayerLayerRGBG,      VK_FORMAT_R16_UNORM,                     DXF(DXGI_FORMAT_R16_UNORM          ), MTLF(MTL::PixelFormatR16Unorm             ), GL_FORMAT_R16,                 FS_C(uint16_t, 1)  },
    { Format::YUV420P,             VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,     DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  3)  },
    { Format::YUV422P,             VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,     DXF(DXGI_FORMAT_YUY2               ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  3)  },
    { Format::YUV444P,             VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,     DXF(DXGI_FORMAT_AYUV               ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  3)  },
    { Format::YUV420P10,           VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,  DXF(DXGI_FORMAT_UNKNOWN            ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint16_t, 3)  },
    { Format::YUV422P10,           VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,  DXF(DXGI_FORMAT_Y210               ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint16_t, 3)  },
    { Format::YUV444P10,           VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,  DXF(DXGI_FORMAT_Y410               ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint16_t, 3)  },
    { Format::NV12,                VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,      DXF(DXGI_FORMAT_NV12               ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint8_t,  3)  },
    { Format::P010LE,              VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,   DXF(DXGI_FORMAT_P010               ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint16_t, 3)  },
    { Format::P016LE,              VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,   DXF(DXGI_FORMAT_P016               ), MTLF(MTL::PixelFormatInvalid              ), GL_FORMAT_INVALID,             FS_C(uint16_t, 3)  },
};

static const FormatElement &GetFormatElement(const Format &format)
{
    size_t index = Format::ValueType(format) & 0xffff;
    return FormatElementTable[index];
}

}

int Format::ComponentCount() const
{
    return StaticSpace::GetFormatElement(*this).ComponentCount;
}

int Format::GetComponent() const
{
	return StaticSpace::GetFormatElement(*this).ComponentCount;
}

Format::operator VkFormat() const
{
    return StaticSpace::GetFormatElement(*this);
}

Format::operator DXGI_FORMAT() const
{
    return StaticSpace::GetFormatElement(*this);
}

Format::operator GL_FORMAT() const
{
    return StaticSpace::GetFormatElement(*this);
}

#ifdef __APPLE__
Format::operator MTL::PixelFormat() const
{
    return StaticSpace::GetFormatElement(*this);
}
#endif

int Format::ElementSize() const
{
    return StaticSpace::GetFormatElement(*this).ElementSize;
}

int Format::BytesPerPixel() const
{
    return ElementSize();
}

size_t Format::Size() const
{
    return ElementSize() * ComponentCount();
}

size_t Format::GetTexelSize() const
{
	return ElementSize() * ComponentCount();
}

Format::Format(VkFormat format) :
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
