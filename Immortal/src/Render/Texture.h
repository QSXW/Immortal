#pragma once

#include "ImmortalCore.h"

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

    enum class Format
    {
        None = 0,
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

        Depth = Depth24Stencil8
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
    
    struct Properties
    {
        Wrap wrap = Wrap::Repeat;
        Filter filter = Filter::Linear;
        bool GenerateMips = true;
        bool SRGB = false;
    };

    struct Description
    {
        Description() = default;
        Description(Texture::Format format, Texture::Wrap wrap = Texture::Wrap::Clamp, Texture::Filter filter = Texture::Filter::Nearest, Texture::Type type = Texture::Type::Texture2D)
            : Format(format), Wrap(wrap), Filter(filter), Type(type) {}

        Texture::Format Format{ Texture::Format::None };
        Texture::Wrap   Wrap{ Texture::Wrap::Clamp };
        Texture::Filter Filter{ Texture::Filter::Nearest };
        Texture::Type   Type{ Texture::Type::Texture2D };

        bool Anisotropic{ true };
    };

public:
    virtual ~Texture() = default;

    virtual uint32_t Width() const
    {
        return 0;
    }

    virtual uint32_t Height() const
    {
        return 0;
    }

    virtual float Ratio() const
    {
        return 0;
    }

    virtual uint64_t Handle() const
    {
        return 0;
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

public:
    static inline bool IsDepthFormat(Texture::Format format)
    {
        switch (format)
        {
            case Format::Depth32F:
            case Format::Depth24Stencil8:
                return true;
            default:
                return false;
        }
    }
};

using Image = Texture;
using SuperTexture = Texture;

class IMMORTAL_API Texture2D : public Texture
{
public:
    static std::shared_ptr<Texture2D> Create(uint32_t width, uint32_t height);

    static std::shared_ptr<Texture2D> Create(const std::string &filepath);

    static std::shared_ptr<Texture2D> Create(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels = 0);

    static std::shared_ptr<Texture2D> Create(const std::string &path, Texture::Wrap wrap, Texture::Filter filter);

    static std::shared_ptr<Texture2D> Create(UINT32 width, UINT32 height, const void *data, Texture::Description &spec);

    static std::shared_ptr<Texture2D> Create(const std::string &path, bool flip, Texture::Wrap wrap, Texture::Filter filter);
};

class IMMORTAL_API TextureCube : public Texture
{
public:
    static std::shared_ptr<TextureCube> Create(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels = 0);
    static std::shared_ptr<TextureCube> Create(const std::string &filepath);
};

using SuperTexture2D  = Texture2D;
using SuperTextureCube = TextureCube;

}
