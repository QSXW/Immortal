#include "impch.h"
#include "Texture.h"

#include "Render/Frame.h"

#include "Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal
{
namespace OpenGL
{

UINT32 InternalCreate(GLenum target, const uint32_t width, const uint32_t height, Texture::DataType & type, int level)
{
    UINT32 texture;

    glCreateTextures(target, 1, &texture);
    glTextureStorage2D(texture, level, type.InternalFromat, width, height);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, texture > 1 ? GL_LINEAR_MIPMAP_LINEAR : type.Filter);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, type.Wrap);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_R, type.Wrap);

    return texture;
}

Texture2D::Texture2D(uint32_t width, uint32_t height) :
    width{ width }, 
    height{ height },
    handle{ 0 },
    ratio{ ncast<float>(width) / ncast<float>(height) }
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

Texture2D::Texture2D(const std::string &path, bool flip, Texture::Wrap wrap, Texture::Filter filter)
{
    LOG::INFO("{0} Loading: {1}", __func__, path);
    Frame frame = Frame(path, flip);

    width = frame.Width();
    height = frame.Height();
    ratio = (float)width / (float)height;

    level = Texture::CalculateMipmapLevels(width, height);
    type = NativeTypeToOpenGl(frame.Type().Format, wrap, filter);
    handle = InternalCreate(GL_TEXTURE_2D, width, height, type, level);

    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, frame.Data());

    if (level > 0)
    {
        glGenerateTextureMipmap(handle);
    }
}

Texture2D::Texture2D(const std::string &path, bool flip) :
    filepath{ path },
    handle{ 0 }
{
    LOG::INFO("{0} Loading: {1}", __func__, path);
    Frame frame = Frame(path, flip);

    width  = frame.Width();
    height = frame.Height();
    ratio  = (float)width / (float)height;

    type = NativeTypeToOpenGl(frame.Type());

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, type.InternalFromat, width, height);
    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, frame.Data());

    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, type.Filter);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);

    level = Texture::CalculateMipmapLevels(width, height);
    if (level > 0)
    {
        glGenerateTextureMipmap(handle);
    }
    LOG::INFO("{0} Completed: {1}", __func__, path);
}

Texture2D::Texture2D(const std::string & path, Texture::Wrap wrap, Texture::Filter filter)
{
    LOG::INFO("{0} Loading: {1}", __func__, path);
    Frame frame = Frame(path);

    width = frame.Width();
    height = frame.Height();
    ratio = (float)width / (float)height;

    type = NativeTypeToOpenGl(frame.Type().Format, wrap, filter);

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, type.InternalFromat, width, height);
    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, frame.Data());

    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, type.Filter);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);

    level = Texture::CalculateMipmapLevels(width, height);
    if (level > 0)
    {
        glGenerateTextureMipmap(handle);
    }
    LOG::INFO("{0} Completed: {1}", __func__, path);
}

Texture2D::Texture2D(const uint32_t width, const uint32_t height, Texture::Description & spec, int levels) :
    width{ width },
    height{ height }
{
    level = (levels > 0) ? levels : Texture::CalculateMipmapLevels(width, height);

    type = NativeTypeToOpenGl(spec);

    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, level, type.InternalFromat, width, height);
        
    // glTextureParameterf(handle, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, level > 1 ? GL_LINEAR_MIPMAP_LINEAR : type.Filter);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);
}


Texture2D::Texture2D(UINT32 width, UINT32 height, const void *data, Texture::Description &spec, INT32 level)
    : width(width), height(height)
{
    level   = (level > 0) ? level : Texture::CalculateMipmapLevels(width, height);
    type    = NativeTypeToOpenGl(spec);
    handle = InternalCreate(GL_TEXTURE_2D, width, height, type, level);

    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, data);

    if (level > 0)
    {
        glGenerateTextureMipmap(handle);
    }
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &handle);
}

void Texture2D::SetData(void * data, uint32_t size)
{
    uint32_t bpp = type.DataFormat == GL_RGBA ? 4 : 3;
    SLASSERT(size == width * height * bpp && "Data must be entire texture!");

    // glTextureStorage2D(handle, 1, mInternalFormat, width, height);
    glTextureSubImage2D(handle, 0, 0, 0, width, height, type.DataFormat, type.BinaryType, data);
}

void Texture2D::Map(uint32_t slot) const
{
    glBindTextureUnit(slot, handle);
}

uint32_t Texture2D::MipLevelCount() const
{
    return uint32_t();
}

void Texture2D::BindImageTexture(bool layered)
{
    glBindImageTexture(0, handle, 0, layered ? GL_TRUE : GL_FALSE, 0, GL_WRITE_ONLY, type.InternalFromat);
}

TextureCube::TextureCube(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels)
{
    this->Create(width, height, spec, levels);
}

void TextureCube::Create(const uint32_t width, const uint32_t height, Texture::Description &description, int levels)
{
    this->width = width;
    this->height = height;
    level = (levels > 0) ? levels : CalculateMipmapLevels(width, height);
    desc = description;

    type = NativeTypeToOpenGl(desc);

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &handle);
    glTextureStorage2D(handle, level, type.InternalFromat, width, height);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, level > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, type.Wrap);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_R, type.Wrap);
}

TextureCube::TextureCube(const std::string & path)
    : width(0), height(0), level(0), handle(0)
{
    // Parameters
    static constexpr uint32_t cubemapSize = 1024;

    // Unfiltered environment cube map (temporary).
    TextureCube envCubeUnfiltered(cubemapSize, cubemapSize, Texture::Description{ Texture::Format::RGBA16F, Texture::Wrap::Clamp, Texture::Filter::Linear });
    // Load & convert Equirectangular Environment Map to a Cubemap texture.
    {
        OpenGL::Shader equirectangleToCubeShader("assets/shaders/equirect2cube_cs.glsl");
        Texture2D envEquirect(path, false, Texture::Wrap::Clamp, Texture::Filter::Linear);

        equirectangleToCubeShader.Map();
        envEquirect.Map(1);
        glBindImageTexture(0, envCubeUnfiltered.Handle(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        glDispatchCompute(envCubeUnfiltered.Width() / 32, envCubeUnfiltered.Height() / 32, 6);
    }
    glGenerateTextureMipmap(envCubeUnfiltered.Handle());

    // Compute pre-filtered specular environment map.
    {
        OpenGL::Shader spmapShader("assets/shaders/spmap_cs.glsl");
        this->Create(cubemapSize, cubemapSize, Texture::Description{ Texture::Format::RGBA16F, Texture::Wrap::Clamp, Texture::Filter::Linear });

        // Copy unfiltered texture to the current
        glCopyImageSubData(envCubeUnfiltered.Handle(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                            this->Handle(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
                            this->Width(), this->Height(), 6);

        spmapShader.Map();
        envCubeUnfiltered.Map();

        // Pre-filter rest of the mip chain.
        const float deltaRoughness = 1.0f / std::max(float(level - 1), 1.0f);
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

uint32_t TextureCube::MipLevelCount() const
{
    return uint32_t();
}

void TextureCube::SetData(void * data, uint32_t size)
{

}

}
}