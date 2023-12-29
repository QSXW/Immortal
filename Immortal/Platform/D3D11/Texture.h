#pragma once

#include "Render/Texture.h"
#include "Common.h"
#include "Image.h"
#include "Descriptor.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class IMMORTAL_API Texture : public SuperTexture, public NonDispatchableHandle
{
public:
    using Super = SuperTexture;
	using Primitive = ID3D11Texture2D;

    D3D11_OPERATOR_HANDLE()
	D3D11_SWAPPABLE(Texture)

public:
	Texture(Device *device = nullptr);

	Texture(Device *device, Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type);

    Texture(Device *device, ComPtr<ID3D11Texture2D> pTexture);

	virtual ~Texture() override;

public:
    operator bool() const
    {
		return handle != nullptr;
    }

    const Descriptor<SRV> &GetDescriptor() const
    {
        return descriptor;
    }

    const Descriptor<UAV> &GetUAV() const
	{
		return uav;
	}

    const Descriptor<RTV> &GetRTV() const
	{
		return rtv;
	}

    const Descriptor<DSV> &GetDSV() const
    {
        return dsv;
    }

    uint32_t GetMipLevels() const
    {
		D3D11_TEXTURE2D_DESC desc{};
		handle->GetDesc(&desc);
		return desc.MipLevels;
    }

    void Swap(Texture &other)
    {
		NonDispatchableHandle::Swap(other);
		std::swap(handle,     other.handle    );
		std::swap(descriptor, other.descriptor);
        std::swap(rtv,        other.rtv       );
        std::swap(dsv,        other.dsv       );
		std::swap(uav,        other.uav       );
    }

protected:
    void ConstructResourceView();

protected:
    Descriptor<SRV> descriptor;

    Descriptor<RTV> rtv;

    Descriptor<DSV> dsv;

    Descriptor<UAV> uav;
};

}
}
