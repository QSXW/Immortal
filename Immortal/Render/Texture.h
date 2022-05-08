#pragma once

#include "Core.h"
#include "Format.h"
#include "Descriptor.h"
#include "Interface/IObject.h"

namespace Immortal
{

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

class IMMORTAL_API Texture : public IObject
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

    enum class Type
    {
        None = 0,
        Texture2D,
        TextureCube
    };

    struct Description
    {
        Description()
        {

        }

        Description(std::initializer_list<Description> &list)
        {

        }

        Description(Wrap wrap, Filter filter = Filter::Nearest, bool mipLevels = true, Texture::Type type = Texture::Type::Texture2D) :
            wrap{ wrap },
            filter{ filter },
            type{ type },
            mipLevels{ mipLevels }
        {

        }

        Description(Format format, Wrap wrap = Wrap::Clamp, Filter filter = Filter::Nearest, bool mipLevels = true, Texture::Type type = Texture::Type::Texture2D) :
            format{ format },
            wrap{ wrap },
            filter{ filter },
            type{ type },
            mipLevels{ mipLevels }
        {

        }

        bool IsDepth() const
        {
            return format == Format::Depth || format == Format::Depth24Stencil8;
        }

        Format format = Format::R8G8B8A8_UNORM;
        Wrap   wrap   = Wrap::Clamp;
        Filter filter = Filter::Nearest;
        Type   type   = Type::Texture2D;

        bool mipLevels = true;
        bool Anisotropic{ true };
    };

public:
    Texture()
    {

    }

    Texture(const std::string &source) :
        source{ source }
    {

    }

    Texture(uint32_t width, uint32_t height, const std::string &source = {}) :
        source{ source },
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

    virtual uint32_t MipLevels() const
    {
        return mipLevels;
    }

    virtual uint32_t LayerCount() const
    {
        return 1;
    }

    virtual void Update(void *data) { }

    virtual void Map(uint32_t slot = 0) { }

    virtual const std::string &Source() const
    {
        return source;
    }

    virtual bool operator==(const Texture &other) const
    {
        return false;
    }

    virtual void BindImageTexture(bool layered) { }

protected:
    std::string source;

    uint32_t width = 0;

    uint32_t height = 0;

    uint32_t mipLevels = 1;
};

using Image = Texture;
using SuperTexture = Texture;

class IMMORTAL_API TextureCube : public virtual Texture
{
public:
    using Super = Texture;

    TextureCube()
    {

    }

    TextureCube(const std::string &source) :
        Super{ source }
    {

    }

    virtual uint32_t LayerCount() const override
    {
        return 6;
    }
};

using SuperTextureCube = TextureCube;

}
