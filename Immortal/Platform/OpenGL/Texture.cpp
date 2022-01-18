#include "impch.h"
#include "Texture.h"

#include "Render/Frame.h"
#include "Shader.h"

namespace Immortal
{
namespace OpenGL
{

uint32_t InternalCreate(GLenum target, const uint32_t width, const uint32_t height, Texture::DataType & type, int level)
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

Texture::Texture(const std::string &path, bool flip, Wrap wrap, Filter filter)
{
    Frame frame = Frame(path);

    width = frame.Width();
    height = frame.Height();

    mipLevels = Texture::CalculateMipmapLevels(width, height);
    type = NativeTypeToOpenGl(frame.Desc().Format, wrap, filter);
    handle = InternalCreate(GL_TEXTURE_2D, width, height, type, mipLevels);

    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, frame.Data());

    if (mipLevels > 0)
    {
        glGenerateTextureMipmap(handle);
    }
}

Texture::Texture(const std::string &path, const Description &description) :
    filepath{ path },
    handle{ 0 }
{
    Frame frame = Frame(path);

    width  = frame.Width();
    height = frame.Height();

    type = NativeTypeToOpenGl(frame.Desc().Format);

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

    width = frame.Width();
    height = frame.Height();

    type = NativeTypeToOpenGl(frame.Desc().Format, wrap, filter);

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

Texture::Texture(const uint32_t width, const uint32_t height, Texture::Description &description, int levels) :
    Super{ width, height }
{
    mipLevels = (levels > 0) ? levels : Texture::CalculateMipmapLevels(width, height);

    type = NativeTypeToOpenGl(description);

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, mipLevels, type.InternalFromat, width, height);
        
    // glTextureParameterf(handle, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : type.Filter);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);
}


Texture::Texture(const uint32_t width, const uint32_t height, const void *data, const Texture::Description &description, int level) :
    Super{ width, height }
{
    mipLevels = (level > 0) ? level : CalculateMipmapLevels(width, height);
    type    = NativeTypeToOpenGl(description);
    handle = InternalCreate(GL_TEXTURE_2D, width, height, type, mipLevels);

    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, data);

    if (mipLevels > 0)
    {
        glGenerateTextureMipmap(handle);
    }
}

Texture::~Texture()
{
    glDeleteTextures(1, &handle);
}

void Texture::SetData(void * data, uint32_t size)
{
    uint32_t bpp = type.DataFormat == GL_RGBA ? 4 : 3;
    SLASSERT(size == width * height * bpp && "Data must be entire texture!");

    // glTextureStorage2D(handle, 1, mInternalFormat, width, height);
    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, data);
}

void Texture::Map(uint32_t slot)
{
    glBindTextureUnit(slot, handle);
}

void Texture::BindImageTexture(bool layered)
{
    glBindImageTexture(0, handle, 0, layered ? GL_TRUE : GL_FALSE, 0, GL_WRITE_ONLY, type.InternalFromat);
}

void Texture::As(Descriptor::Super *superDescriptors, size_t index)
{
    Descriptor *descriptors = rcast<Descriptor *>(superDescriptors);
    descriptors[index] = handle;
}

TextureCube::TextureCube(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels)
{
    this->Create(width, height, spec, levels);
}

void TextureCube::Create(const uint32_t width, const uint32_t height, Texture::Description &description, int levels)
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

TextureCube::TextureCube(const std::string &path)
{
    // Parameters
    static constexpr uint32_t cubemapSize = 1024;

    // Unfiltered environment cube map (temporary).
    TextureCube envCubeUnfiltered(cubemapSize, cubemapSize, Texture::Description{ Format::RGBA16F, Wrap::Clamp, Filter::Linear });
    // Load & convert Equirectangular Environment Map to a Cubemap texture.
    {
        OpenGL::Shader equirectangleToCubeShader("assets/shaders/equirect2cube_cs.glsl", Shader::Type::Compute);
        OpenGL::Texture envEquirect(path, false, Wrap::Clamp, Filter::Linear);

        equirectangleToCubeShader.Map();
        envEquirect.Map(1);
        glBindImageTexture(0, envCubeUnfiltered.Handle(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        glDispatchCompute(envCubeUnfiltered.Width() / 32, envCubeUnfiltered.Height() / 32, 6);
    }
    glGenerateTextureMipmap(envCubeUnfiltered.Handle());

    // Compute pre-filtered specular environment map.
    {
        OpenGL::Shader spmapShader("assets/shaders/spmap_cs.glsl", Shader::Type::Compute);
        this->Create(cubemapSize, cubemapSize, Texture::Description{ Format::RGBA16F, Wrap::Clamp, Filter::Linear });

        // Copy unfiltered texture to the current
        glCopyImageSubData(envCubeUnfiltered.Handle(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                            this->Handle(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                            this->Width(), this->Height(), 6);

        spmapShader.Map();
        envCubeUnfiltered.Map();

        // Pre-filter rest of the mip chain.
        const float deltaRoughness = 1.0f / std::max(float(mipLevels - 1), 1.0f);
        for (int level = 1, size = cubemapSize / 2; level <= level; level++, size /= 2)
        {
            const GLuint numGroups = std::max(1, size / 32);
            glBindImageTexture(0, this->Handle(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            glProgramUniform1f(spmapShader.Handle(), 0, level * deltaRoughness);
            glDispatchCompute(numGroups, numGroups, 6);
        }
    }
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

void TextureCube::SetData(void * data, uint32_t size)
{

}


}
}
