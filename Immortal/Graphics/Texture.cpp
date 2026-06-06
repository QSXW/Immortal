#include "Texture.h"

namespace Immortal
{

Texture::Texture() :
    _format{ Format::None },
    _width{},
    _height{},
    _mipLevels{},
    _arrayLayers{}
{

}

const Format &Texture::GetFormat() const
{
	return _format;
}

const uint32_t &Texture::GetWidth() const
{
	return _width;
}

const uint32_t &Texture::GetHeight() const
{
	return _height;
}

const uint16_t &Texture::GetMipLevels() const
{
	return _mipLevels;
}

const uint16_t &Texture::GetArrayLayers() const
{
	return _arrayLayers;
}

uint32_t Texture::GetRatio() const
{
	return (float)(GetWidth()) / (float)(GetHeight());
}

void Texture::SetMeta(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers)
{
	_format      = format;
	_width       = width;
	_height      = height;
	_mipLevels   = mipLevels;
	_arrayLayers = arrayLayers;
}

}
