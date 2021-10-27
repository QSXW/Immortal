#pragma once

#include "ImmortalCore.h"

#include "Render/Texture.h"

#include <glad/glad.h>

namespace Immortal
{
namespace OpenGL
{

static UINT32 InternalCreate(GLenum target, const uint32_t width, const uint32_t height, Texture::DataType &spec, int levels = 0);

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

static Texture::DataType NativeTypeToOpenGl(Texture::Format format, Texture::Wrap wrap = Texture::Wrap::Clamp, Texture::Filter filter = Texture::Filter::Linear)
{
    Texture::DataType data{};

    data.Wrap = wrap == Texture::Wrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    data.Filter = filter == Texture::Filter::Linear ? GL_LINEAR : GL_NEAREST;
    switch (format)
    {
    case Texture::Format::RedInterger:
        data.InternalFromat = GL_R32I;
        data.DataFormat = GL_RED_INTEGER;
        data.BinaryType = GL_INT;
        break;

    case Texture::Format::RGBA:
    case Texture::Format::RGBA8:
        data.InternalFromat = GL_RGBA8;
        data.DataFormat = GL_RGBA;
        data.BinaryType = GL_UNSIGNED_BYTE;
        break;

    case Texture::Format::RGB:
    case Texture::Format::RGB8:
        data.InternalFromat = GL_RGB8;
        data.DataFormat = GL_RGB;
        data.BinaryType = GL_UNSIGNED_BYTE;
        break;

    case Texture::Format::RG16F:
        data.InternalFromat = GL_RG16F;
        data.DataFormat = GL_RG;
        data.BinaryType = GL_FLOAT;
        break;

    case Texture::Format::RGBA16F:
        data.InternalFromat = GL_RGBA16F;
        data.DataFormat = GL_RGBA;
        data.BinaryType = GL_FLOAT;
        break;

    case Texture::Format::RGB16F:
        data.InternalFromat = GL_RGB16F;
        data.DataFormat = GL_RGB;
        data.BinaryType = GL_FLOAT;
        break;

    case Texture::Format::RGBA32F:
        data.InternalFromat = GL_RGBA32F;
        data.DataFormat = GL_RGBA;
        data.BinaryType = GL_FLOAT;
        break;

    case Texture::Format::RGB32F:
        data.InternalFromat = GL_RGB32F;
        data.DataFormat = GL_RGB;
        data.BinaryType = GL_FLOAT;
        break;

    case Texture::Format::Depth24Stencil8:
        data.InternalFromat = GL_DEPTH24_STENCIL8;
        break;

    default:
        SLASSERT(false && "Invalid Texture Format.");
        break;
    }

    return data;
}

static Texture::DataType NativeTypeToOpenGl(Texture::Description &spec)
{
    return NativeTypeToOpenGl(spec.Format, spec.Wrap, spec.Filter);
}

class Texture2D : public SuperTexture2D
{
public:
    Texture2D(uint32_t width, uint32_t height);

    Texture2D(const std::string& path, bool flip = false);

    Texture2D(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels);

    Texture2D(const std::string & path, bool flip, Texture::Wrap wrap, Texture::Filter filter);

    Texture2D(const std::string & path, Texture::Wrap wrap, Texture::Filter filter);

    Texture2D(UINT32 width, UINT32 height, const void * data, Texture::Description & spec, INT32 level = 0);

    virtual ~Texture2D();

    virtual uint32_t Width() const override
    { 
        return width;
    }

    virtual uint32_t Height() const override
    { 
        return height;
    }

    virtual uint64_t Handle() const override
    { 
        return ncast<uint64_t>(handle); 
    }

    virtual void SetData(void* data, uint32_t size) override;

    virtual void Map(uint32_t slot = 0) override;

    virtual void BindImageTexture(bool layered = false) override;

    virtual uint32_t MipLevelCount() const override;

    virtual bool operator==(const Texture& other) const override
    {
        return handle == ((Texture2D&)other).handle;
    }
    virtual float Ratio() const override
    {
        return ratio;
    }

    virtual const char *Path() const override
    { 
        return filepath.c_str();
    }

private:
    std::string filepath;

    uint32_t width;

    uint32_t height;

    uint32_t handle;

    int32_t level;

    float ratio;

    Texture::DataType type;
};

class TextureCube : public SuperTextureCube
{
public:
    TextureCube(const uint32_t width, const uint32_t height, Texture::Description&, int levels = 0);

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

    virtual uint32_t MipLevelCount() const override;

    virtual void SetData(void *data, uint32_t size) override;

    virtual const char *Path() const override
    { 
        return filepath.c_str();
    }

    virtual uint64_t Handle() const override
    { 
        return handle;
    }

    virtual bool operator==(const Texture& other) const override
    {
        return handle == ((TextureCube&)other).handle;
    }

    void Create(const uint32_t width, const uint32_t height, Texture::Description &description, int levels = 0);

    virtual void BindImageTexture(bool layered = false) override;

private:
    uint32_t handle{ 0 };

    uint32_t width{ 0 };

    uint32_t height{ 0 };

    int32_t level{ 0 };

    Description desc;

    DataType type;

    std::string filepath;
};

}
}
