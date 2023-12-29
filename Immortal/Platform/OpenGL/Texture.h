#pragma once

#include "Render/Texture.h"
#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

class IMMORTAL_API Texture : public SuperTexture, public Handle
{
public:
    using Super = SuperTexture;
	GLCPP_SWAPPABLE(Texture)

	struct DataType
	{
		GL_FORMAT SizedFormat;
		GL_FORMAT BaseFormat;
		GL_FORMAT BinaryType;
		GLFilter  Filter;
		GLSampler Wrap;
	};

public:
	Texture();

	Texture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type);

    virtual ~Texture() override;

public:
    GL_FORMAT GetFormat() const
    {
		return dataType.SizedFormat;
    }

	GL_FORMAT GetBaseFormat() const
    {
		return dataType.BaseFormat;
    }

    GL_FORMAT GetBinaryFormat() const
    {
		return dataType.BinaryType;
    }

	operator bool() const
	{
		return handle != Handle::Invalid;
	}

	void Swap(Texture &other)
	{
		Handle::Swap(other);
		std::swap(dataType, other.dataType);
	}

protected:
    DataType dataType;
};

Texture::DataType NativeTypeToOpenGl(Format format);

}
}
