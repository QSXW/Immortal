#pragma once

#include "Core.h"
#include "Format.h"
#include "Descriptor.h"

namespace Immortal
{

class IMMORTAL_API Texture
{
public:
    template<class T>
    static constexpr T CalculateMipmapLevels(T width, T height)
    {
        T levels = 1;
        while ((width >> levels) && (height >> levels)) {
            levels++;
        }
        return levels;
    }

public:
    enum class ViewType
    {
        _1D,
        _2D,
        _3D,
        Cube,
        Array1D,
        Array2D,
        Array3D
    };

    struct DataType
    {
        int InternalFromat;
        int DataFormat;
        int BinaryType;
        int Filter;
        int Wrap;
    };

    enum class Wrap
    {
        None = 0,
        Clamp,
        Repeat,
        Wrap = Repeat,
        Mirror,
        BorderColor
    };

    enum class Filter
    {
        None = 0,
        Linear,
        Bilinear,
        Nearest,
        Anisotropic
    };

    enum class Type
    {
        None = 0,
        Texture2D,
        TextureCube
    };

    struct Description
    {
        Description() = default;

        Description(Format format, Texture::Wrap wrap = Texture::Wrap::Clamp, Texture::Filter filter = Texture::Filter::Nearest, Texture::Type type = Texture::Type::Texture2D) :
            Format{ format }, Wrap{ wrap }, Filter{ filter }, Type{ type }
        {
        }

        template <class T>
        T BaseFromat() const
        {
            return Map::BaseFormat<T>(Format);
        }

        auto FormatSize() const
        {
            return Map::FormatSize(Format);
        }

        auto ComponentCount() const
        {
            return Map::FormatComponentCount(Format);
        }

        bool IsDepth() const 
        {
            return Format == Format::Depth;
        }

        Format Format = Format::R8G8B8A8_UNORM;
        Wrap   Wrap   = Wrap::Clamp;
        Filter Filter = Filter::Nearest;
        Type   Type   = Type::Texture2D;

        bool Anisotropic{ true };
    };

public:
    Texture()
    {

    }

    Texture(uint32_t width, uint32_t height) :
        width{ width },
        height{ height },
        mipLevels{ CalculateMipmapLevels(width, height) }
    {

    }

    virtual ~Texture() = default;

    uint32_t Width() const
    {
        return width;
    }

    uint32_t Height() const
    {
        return height;
    }

    float Ratio() const
    {
        return ncast<float>(width) / ncast<float>(height);
    }

    void Update(uint32_t w, uint32_t h)
    {
        width     = w;
        height    = h;
        mipLevels = CalculateMipmapLevels(w, h);
    }

    virtual operator uint64_t() const
    {
        return 0;
    }

    virtual void As(Descriptor *descriptor, size_t index)
    {

    }

    virtual uint32_t MipLevelCount() const
    {
        return 0;
    }

    virtual void SetData(void *data, uint32_t size) { }

    virtual void Map(uint32_t slot = 0) { }

    virtual const char *Path() const
    {
        return nullptr;
    }

    virtual bool operator==(const Texture &other) const
    {
        return false;
    }

    virtual void BindImageTexture(bool layered) { }

protected:
    uint32_t width{ 0 };

    uint32_t height{ 0 };

    uint32_t mipLevels{ 1 };
};

using Image = Texture;
using SuperTexture = Texture;

class IMMORTAL_API TextureCube : public Texture
{

};

using SuperTextureCube = TextureCube;

}
