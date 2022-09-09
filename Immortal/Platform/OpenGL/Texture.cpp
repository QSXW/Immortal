#include "Texture.h"

#include "Render/Frame.h"
#include "Shader.h"

namespace Immortal
{
namespace OpenGL
{

uint32_t InternalCreate(GLenum target, const uint32_t width, const uint32_t height, Texture::DataType &type, int level)
{
    uint32_t texture;

    glCreateTextures(target, 1, &texture);
    glTextureStorage2D(texture, level, type.InternalFromat, width, height);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, texture > 1 ? GL_LINEAR_MIPMAP_LINEAR : type.Filter);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, type.Wrap);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_R, type.Wrap);

    return texture;
}

Texture::Texture(uint32_t width, uint32_t height) :
    Super{ width, height },
    handle{ 0 }
{
    type.InternalFromat = GL_RGBA8;
    type.DataFormat     = GL_RGBA;
    type.BinaryType     = GL_UNSIGNED_BYTE;

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, type.InternalFromat, width, height);

    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::Texture(const std::string &path, bool flip, Wrap wrap, Filter filter) :
    Super{ path }
{
    Frame frame = Frame(path);

    Vision::Picture picture = frame.GetPicture();
    width  = picture.desc.width;
    height = picture.desc.height;

    mipLevels = Texture::CalculateMipmapLevels(width, height);
    type = NativeTypeToOpenGl(picture.desc.format, wrap, filter);
    handle = InternalCreate(GL_TEXTURE_2D, width, height, type, mipLevels);

    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, frame.Data());

    Blit();
}

Texture::Texture(const std::string &path, const Description &description) :
    Super{ path },
    handle{ 0 }
{
    Frame frame = Frame(path);
    
    Vision::Picture picture = frame.GetPicture();
    
    width  = picture.desc.width;
    height = picture.desc.height;

    type = NativeTypeToOpenGl(picture.desc.format);

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, type.InternalFromat, width, height);
    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, frame.Data());

    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, type.Filter);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);

    mipLevels = Texture::CalculateMipmapLevels(width, height);
    if (mipLevels > 0)
    {
        glGenerateTextureMipmap(handle);
    }
}

Texture::Texture(const std::string &path, Wrap wrap, Filter filter)
{
    Frame frame = Frame(path);

    Vision::Picture picture = frame.GetPicture();

    width  = picture.desc.width;
    height = picture.desc.height;

    type = NativeTypeToOpenGl(picture.desc.format, wrap, filter);

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, type.InternalFromat, width, height);
    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, frame.Data());

    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, type.Filter);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);

    mipLevels = Texture::CalculateMipmapLevels(width, height);
	Blit();
}

Texture::Texture(const uint32_t width, const uint32_t height, Texture::Description &description) :
    Super{ width, height }
{
    type = NativeTypeToOpenGl(description);

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, mipLevels, type.InternalFromat, width, height);

    glTextureParameterf(handle, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : type.Filter);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);
}

Texture::Texture(const uint32_t width, const uint32_t height, const void *data, const Texture::Description &description) :
    Super{ width, height }
{
    type   = NativeTypeToOpenGl(description);
    handle = InternalCreate(GL_TEXTURE_2D, width, height, type, mipLevels);

    if (data)
    {
		glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, data);
    }

    Blit();
}

Texture::~Texture()
{
    glDeleteTextures(1, &handle);
}

void Texture::Update(const void *data, uint32_t pitchX)
{
    uint32_t bpp = type.DataFormat == GL_RGBA ? 4 : 3;

    glTextureSubImage2D(handle, 0, pitchX, 0, width, height, type.DataFormat, type.BinaryType, data);
}

void Texture::Map(uint32_t slot)
{
    glBindTextureUnit(slot, handle);
}

void Texture::BindImageTexture(bool layered)
{
    glBindImageTexture(0, handle, 0, layered ? GL_TRUE : GL_FALSE, 0, GL_WRITE_ONLY, type.InternalFromat);
}

void Texture::Blit()
{
    if (mipLevels > 1)
    {
		glGenerateTextureMipmap(handle);
    }
}

void Texture::As(DescriptorBuffer *descriptorBuffer, size_t index)
{
    auto descriptor = descriptorBuffer->DeRef<uint32_t>(index);
	descriptor[0] = handle;
}

TextureCube::TextureCube(const uint32_t width, const uint32_t height, const Texture::Description &description, int levels)
{
    this->Create(width, height, description, levels);
}

void TextureCube::Create(const uint32_t width, const uint32_t height, const Texture::Description &description, int levels)
{
    this->width = width;
    this->height = height;
    mipLevels = (levels > 0) ? levels : CalculateMipmapLevels(width, height);
    desc = description;

    type = NativeTypeToOpenGl(desc);

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &handle);
    glTextureStorage2D(handle, mipLevels, type.InternalFromat, width, height);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_R, type.Wrap);
}

TextureCube::TextureCube(const std::string &path) :
    Super{ path }
{

}

TextureCube::~TextureCube()
{
    glDeleteTextures(1, &handle);
}

void TextureCube::BindImageTexture(bool layered)
{
    glBindImageTexture(0, handle, 0, layered ? GL_TRUE : GL_FALSE, 0, GL_WRITE_ONLY, type.InternalFromat);
}

void TextureCube::Map(uint32_t slot) const
{
    glBindTextureUnit(slot, handle);
}

void TextureCube::Update(const void *data, uint32_t pitchX)
{

}


}
}
