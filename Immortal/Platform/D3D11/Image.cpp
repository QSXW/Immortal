#include "Image.h"
#include "Device.h"

namespace Immortal
{
namespace D3D11
{

Image::Image() :
    format{}
{

}

Image::Image(Format format) :
    format{ format }
{

}

Image::Image(Device *device, uint32_t usage, Format format, uint32_t width, uint32_t height, const void *data, uint32_t mipLevels) :
    format{ format }
{
    Create(device, usage, width, height, data, mipLevels);
}

Image::~Image()
{

}

void Image::Create(Device *device, uint32_t usage, uint32_t width, uint32_t height, const void *data, uint32_t mipLevels)
{

}

}
}
