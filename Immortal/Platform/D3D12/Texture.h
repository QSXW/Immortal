#pragma once

#include "Render/Texture.h"

#include "Common.h"
#include "Descriptor.h"
#include "Resource.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class RenderContext;
class IMMORTAL_API Texture : public SuperTexture, public Resource, public NonDispatchableHandle
{
public:
    using Super = SuperTexture;

public:
    Texture(Device *device, ID3D12Resource *resource, D3D12_RESOURCE_STATES state);

    Texture(Device *device, Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type);

    virtual ~Texture() override;

    D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor(uint32_t subresource = 0);

    D3D12_CPU_DESCRIPTOR_HANDLE GetUAVDescriptor(uint32_t subresource);

public:
    DXGI_FORMAT GetFormat() const
    {
		return format;
    }

protected:
    void Construct(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type);

    void ConstructShaderResourceView();

protected:
    DXGI_FORMAT format;

    Descriptor descriptor;

    Descriptor uav;
};

}
}
