#include "Texture.h"

namespace Immortal
{
namespace OpenGL
{

Texture::DataType NativeTypeToOpenGl(Format format)
{
	Texture::DataType data{};

	data.SizedFormat = format;
	switch (data.SizedFormat)
	{
		case GL_FORMAT_R8:
		case GL_FORMAT_RG8:
		case GL_FORMAT_RGB8:
		case GL_FORMAT_RGBA8:
			data.BinaryType = GL_FORMAT_UNSIGNED_BYTE;
			break;

        case GL_FORMAT_R16:
		case GL_FORMAT_RG16:
		case GL_FORMAT_RGB16:
		case GL_FORMAT_RGBA16:
			data.BinaryType = GL_FORMAT_UNSIGNED_SHORT;
			break;

		case GL_FORMAT_R16F:
		case GL_FORMAT_RG16F:
		case GL_FORMAT_RGB16F:
		case GL_FORMAT_RGBA16F:
			data.BinaryType = GL_FORMAT_FLOAT;
			break;

        case GL_FORMAT_R32I:
		case GL_FORMAT_RG32I:
		case GL_FORMAT_RGB32I:
		case GL_FORMAT_RGBA32I:
			data.BinaryType = GL_FORMAT_INT;
			break;

        case GL_FORMAT_R32UI:
		case GL_FORMAT_RG32UI:
		case GL_FORMAT_RGB32UI:
		case GL_FORMAT_RGBA32UI:
			data.BinaryType = GL_FORMAT_UNSIGNED_INT;
			break;

		case GL_FORMAT_R32F:
		case GL_FORMAT_RG32F:
		case GL_FORMAT_RGB32F:
		case GL_FORMAT_RGBA32F:
			data.BinaryType = GL_FORMAT_FLOAT;
			break;

		case GL_FORMAT_DEPTH:
		case GL_FORMAT_DEPTH24_STENCIL8:
		case GL_FORMAT_DEPTH32F_STENCIL8:
			break;

		default:
			SLASSERT(false && "Invalid Texture Format.");
			break;
	}

	switch (data.SizedFormat)
	{
		case GL_FORMAT_R8:
		case GL_FORMAT_R8_SNORM:
		case GL_FORMAT_R8I:
		case GL_FORMAT_R8UI:
		case GL_FORMAT_R16:
		case GL_FORMAT_R16_SNORM:
		case GL_FORMAT_R16I:
		case GL_FORMAT_R16UI:
		case GL_FORMAT_R32UI:
		case GL_FORMAT_R16F:
		case GL_FORMAT_R32F:
			data.BaseFormat = GL_FORMAT_RED;
			break;

		case GL_FORMAT_R32I:
			data.BaseFormat = GL_FORMAT_RED_INTEGER;
			break;

		case GL_FORMAT_RG8:
		case GL_FORMAT_RG8_SNORM:
		case GL_FORMAT_RG8I:
		case GL_FORMAT_RG8UI:
		case GL_FORMAT_RG16:
		case GL_FORMAT_RG16_SNORM:
		case GL_FORMAT_RG16I:
		case GL_FORMAT_RG16UI:
		case GL_FORMAT_RG16F:
		case GL_FORMAT_RG32I:
		case GL_FORMAT_RG32UI:
		case GL_FORMAT_RG32F:
			data.BaseFormat = GL_FORMAT_RG;
			break;

		case GL_FORMAT_RGB8:
		case GL_FORMAT_RGB8_SNORM:
		case GL_FORMAT_RGB8I:
		case GL_FORMAT_RGB8UI:
		case GL_FORMAT_RGB16:
		case GL_FORMAT_RGB16_SNORM:
		case GL_FORMAT_RGB16I:
		case GL_FORMAT_RGB16UI:
		case GL_FORMAT_RGB16F:
		case GL_FORMAT_RGB32I:
		case GL_FORMAT_RGB32UI:
		case GL_FORMAT_RGB32F:
			data.BaseFormat = GL_FORMAT_RGB;
			break;

		case GL_FORMAT_RGBA8:
		case GL_FORMAT_RGBA8_SNORM:
		case GL_FORMAT_RGBA8I:
		case GL_FORMAT_RGBA8UI:
		case GL_FORMAT_RGBA16:
		case GL_FORMAT_RGBA16_SNORM:
		case GL_FORMAT_RGBA16I:
		case GL_FORMAT_RGBA16UI:
		case GL_FORMAT_RGBA16F:
		case GL_FORMAT_RGBA32I:
		case GL_FORMAT_RGBA32UI:
		case GL_FORMAT_RGBA32F:
			data.BaseFormat = GL_FORMAT_RGBA;
			break;

		case GL_FORMAT_DEPTH:
		case GL_FORMAT_DEPTH24_STENCIL8:
		case GL_FORMAT_DEPTH32F_STENCIL8:
			break;

		default:
			SLASSERT(false && "Invalid Texture Format.");
			break;
	}

	return data;
}

static uint32_t CreateTexture(GLenum target, uint32_t width, uint32_t height, const Texture::DataType &type, int mipLevels)
{
    uint32_t texture;

    glCreateTextures(target, 1, &texture);
	glTextureStorage2D(texture, mipLevels, type.SizedFormat, width, height);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : type.Filter);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, type.Filter);

    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, type.Wrap);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, type.Wrap);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_R, type.Wrap);

    return texture;
}

Texture::Texture() :
    Handle{}
{

}

Texture::Texture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type) :
    Handle{}
{
	SetMeta(format, width, height, mipLevels, arrayLayers);
	GLenum target = GL_TEXTURE_2D;
	if (arrayLayers == 6)
	{
		target = GL_TEXTURE_CUBE_MAP;
	}
	else if (arrayLayers > 1)
	{
		target = GL_TEXTURE_2D_ARRAY;
	}

	dataType = NativeTypeToOpenGl(format);

	glCreateTextures(target, 1, &handle);
	glTextureStorage2D(handle, mipLevels, dataType.SizedFormat, width, height);
}

Texture::~Texture()
{
	if (handle)
	{
		glDeleteTextures(1, &handle);
		handle = 0;
	}
}

}
}
