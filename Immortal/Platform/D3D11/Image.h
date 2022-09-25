#pragma once

#include "Format.h"
#include "Common.h"
#include "Descriptor.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class Image : public IObject
{
public:
    using Primitive = ID3D11Texture2D;
    D3D11_OPERATOR_HANDLE()

    Image();

    Image(Format format);

    Image(Device *device, uint32_t usage, Format format, uint32_t width, uint32_t height, const void *data, uint32_t mipLevels = 1);

    ~Image();

    void Create(Device *device, uint32_t usage, uint32_t width, uint32_t height, const void *data, uint32_t mipLevels = 1);

public:
    Format format;
};

}
}
