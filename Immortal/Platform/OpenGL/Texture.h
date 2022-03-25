#pragma once

#include "Render/Texture.h"
#include "Common.h"
#include "Descriptor.h"

namespace Immortal
{
namespace OpenGL
{

static uint32_t InternalCreate(GLenum target, const uint32_t width, const uint32_t height, Texture::DataType &spec, int levels = 0);

static inline GLenum TextureTarget(bool multisampled)
{
    return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

static inline void CreateTexture(bool multisampled, uint32_t *outTextureID, uint32_t count)
{
    glCreateTextures(TextureTarget(multisampled), count, outTextureID);
}

static inline void BindTexture(bool multisampled, uint32_t id)
{
    glBindTexture(TextureTarget(multisampled), id);
}

static Texture::DataType NativeTypeToOpenGl(Format format, Wrap wrap = Wrap::Clamp, Filter filter = Filter::Linear)
{
    Texture::DataType data{};

    data.Wrap = wrap == Wrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    data.Filter = filter == Filter::Linear ? GL_LINEAR : GL_NEAREST;
    switch (Format::ValueType(format))
    {
    case Format::R32:
        data.InternalFromat = GL_R32I;
        data.DataFormat = GL_RED_INTEGER;
        data.BinaryType = GL_INT;
        break;

    case Format::RGBA8:
        data.InternalFromat = GL_RGBA8;
        data.DataFormat = GL_RGBA;
        data.BinaryType = GL_UNSIGNED_BYTE;
        break;

    case Format::RGB8:
        data.InternalFromat = GL_RGB8;
        data.DataFormat = GL_RGB;
        data.BinaryType = GL_UNSIGNED_BYTE;
        break;

    case Format::RG16F:
        data.InternalFromat = GL_RG16F;
        data.DataFormat = GL_RG;
        data.BinaryType = GL_FLOAT;
        break;

    case Format::RGBA16F:
        data.InternalFromat = GL_RGBA16F;
        data.DataFormat = GL_RGBA;
        data.BinaryType = GL_FLOAT;
        break;

    case Format::RGBA32F:
        data.InternalFromat = GL_RGBA32F;
        data.DataFormat = GL_RGBA;
        data.BinaryType = GL_FLOAT;
        break;

    case Format::RGB32F:
        data.InternalFromat = GL_RGB32F;
        data.DataFormat = GL_RGB;
        data.BinaryType = GL_FLOAT;
        break;

    case Format::Depth24Stencil8:
        data.InternalFromat = GL_DEPTH24_STENCIL8;
        data.BinaryType = GL_FLOAT;
        break;

    case Format::Depth32F:
        data.InternalFromat = GL_DEPTH32F_STENCIL8;
        data.BinaryType = GL_FLOAT;
        break;

    default:
        SLASSERT(false && "Invalid Texture Format.");
        break;
    }

    return data;
}

static inline Texture::DataType NativeTypeToOpenGl(const Texture::Description &spec)
{
    return NativeTypeToOpenGl(spec.format, spec.wrap, spec.filter);
}

class Texture : public SuperTexture
{
public:
    using Super = SuperTexture;

public:
    Texture(uint32_t width, uint32_t height);

    Texture(const std::string &path, const Description &description);

    Texture(const uint32_t width, const uint32_t height, Description &description, int levels);

    Texture(const std::string & path, bool flip, Wrap wrap, Filter filter);

    Texture(const std::string & path, Wrap wrap, Filter filter);

    Texture(const uint32_t width, const uint32_t height, const void *data, const Texture::Description &description, int level = 0);

    virtual ~Texture();

    virtual operator uint64_t() const override
    {
        return ncast<uint64_t>(handle);
    }

    virtual void SetData(void* data, uint32_t size) override;

    virtual void Map(uint32_t slot = 0) override;

    virtual void BindImageTexture(bool layered = false) override;

    virtual bool operator==(const Super &super) const override
    {
        auto other = dcast<const Texture *>(&super);
        return handle == other->handle;
    }

    virtual const char *Path() const override
    {
        return filepath.c_str();
    }

    uint32_t Handle() const
    {
        return ncast<uint64_t>(handle);
    }

    virtual void As(Descriptor::Super *descriptors, size_t index) override;

private:
    std::string filepath;

    uint32_t handle;

    Texture::DataType type;
};

class TextureCube : public SuperTextureCube
{
public:
    TextureCube(const uint32_t width, const uint32_t height, const Texture::Description&, int levels = 0);

    TextureCube(const std::string& path);

    virtual ~TextureCube();

    virtual void Map(uint32_t slot = 0) const;

    virtual uint32_t Width() const
    {
        return width;
    }

    virtual uint32_t Height() const
    {
        return height;
    }

    virtual float Ratio() const
    {
        return (float)width / (float)height;
    }

    virtual void SetData(void *data, uint32_t size) override;

    virtual const char *Path() const override
    {
        return filepath.c_str();
    }

    virtual operator uint64_t() const override
    {
        return handle;
    }

    virtual bool operator==(const Texture& other) const override
    {
        return handle == ((TextureCube&)other).handle;
    }

    void Create(const uint32_t width, const uint32_t height, const Texture::Description &description, int levels = 0);

    virtual void BindImageTexture(bool layered = false) override;

    uint64_t Handle() const
    {
        return handle;
    }

private:
    uint32_t handle{ 0 };

    Description desc;

    DataType type;

    std::string filepath;
};

}
}
