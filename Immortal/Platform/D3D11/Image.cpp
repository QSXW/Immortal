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
    D3D11_TEXTURE2D_DESC desc = {
        .Width          = width,
        .Height         = height,
        .MipLevels      = mipLevels,
        .ArraySize      = 1,
        .Format         = format,
        .SampleDesc     = { .Count = 1, .Quality = 0, },
        .Usage          = D3D11_USAGE_DEFAULT,
        .BindFlags      = usage,
        .CPUAccessFlags = 0,
        .MiscFlags      = mipLevels > 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0U,
    };

    D3D11_SUBRESOURCE_DATA subresource = {
        .pSysMem          = data,
	    .SysMemPitch      = width,
        .SysMemSlicePitch = 0,
    };

    device->CreateTexture2D(&desc, data ? &subresource : nullptr, &handle);
}

}
}
