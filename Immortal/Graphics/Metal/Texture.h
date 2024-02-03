#pragma once

#include "Graphics/Texture.h"
#include "Common.h"
#include "Handle.h"
#include "Metal/MTLTexture.hpp"

namespace Immortal
{
namespace Metal
{

class Device;
class IMMORTAL_API Texture : public SuperTexture, public Handle<MTL::Texture>
{
public:
    using Super = SuperTexture;
    METAL_SWAPPABLE(Texture)

public:
    Texture();

    Texture(MTL::Texture *texture);

    Texture(Device *device, Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type);

    virtual ~Texture() override;

public:
    void Swap(Texture &other)
    {
        Super::Swap(other);
        Handle::Swap(other);
    }
};

}
}
