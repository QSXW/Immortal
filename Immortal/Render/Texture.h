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
    requires std::is_integral_v<T>
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

        Description(Wrap wrap, Filter filter = Filter::Nearest, bool mipLevels = true) :
            wrap{ wrap },
            filter{ filter },
            mipLevels{ mipLevels }
        {

        }

        Description(Format format, Wrap wrap = Wrap::Clamp, Filter filter = Filter::Nearest, bool mipLevels = true) :
            format{ format },
            wrap{ wrap },
            filter{ filter },
            mipLevels{ mipLevels }
        {

        }

        bool IsDepth() const
        {
            return format == Format::Depth32F || format == Format::Depth24Stencil8;
        }

        Format format = Format::RGBA8;
        Wrap   wrap   = Wrap::Clamp;
        Filter filter = Filter::Nearest;

        bool mipLevels = true;
        bool Anisotropic{ true };
    };

public:
    static Texture *CreateInstance(const std::string& filepath, const Description &description = {});

public:
    Texture()
    {

    }

    Texture(uint32_t width, uint32_t height, bool isMipLevel = false) :
        width{ width },
        height{ height },
        mipLevels{ isMipLevel ? CalculateMipmapLevels(width, height) : 1 }
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

    void SetProperties(uint32_t _width, uint32_t _height, bool isMipLevel = false)
    {
        width     = _width;
        height    = _height;
        mipLevels = isMipLevel ? CalculateMipmapLevels(width, height) : 1;
    }

    virtual operator uint64_t() const
    {
        return 0;
    }

    virtual void As(DescriptorBuffer *descriptor, size_t index)
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

    virtual void Update(const void *data, uint32_t pitchX = 0) {}

    virtual void CopyImage(const uint8_t *const *data, const int *pLinesize, int height, int planes) {}

    virtual void PlatformSpecifiedUpdate(Anonymous resource, uint32_t subresource) {}

    virtual void Blit() {}

    virtual void Map(uint32_t slot = 0) { }

    virtual bool operator==(const Texture &other) const
    {
        return false;
    }

    virtual void BindImageTexture(bool layered) { }

protected:
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

    virtual uint32_t LayerCount() const override
    {
        return 6;
    }
};

using SuperTextureCube = TextureCube;

namespace Interface
{
    using Texture = SuperTexture;
    using TextureCube = SuperTextureCube;
}

}
