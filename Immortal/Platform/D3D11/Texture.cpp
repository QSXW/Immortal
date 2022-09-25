#include "Texture.h"

#include "Render/Frame.h"
#include "Device.h"
#include "Buffer.h"

namespace Immortal
{
namespace D3D11
{

Texture::Texture(Device *device, const std::string &filepath, const Description &description) :
    device{ device },
    Super{ filepath }
{
    Frame frame{ filepath };

    Vision::Picture picture = frame.GetPicture();
    if (!picture.Available())
    {
        return;
    }

    Super::Update(picture.desc.width, picture.desc.height);
    format = picture.desc.format;
	__Create(picture.Data());
}

Texture::Texture(Device *device, uint32_t width, uint32_t height, const void *data, const Description &description) :
    device{ device },
    Super{ width, height }
{
	if (!description.mipLevels)
	{
		mipLevels = 1;
	}

    format = description.format;
	__Create(data);
}

Texture::~Texture()
{

}

void Texture::__Create(const void *data)
{
	UINT flags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	if (mipLevels > 1)
	{
		flags |= D3D11_BIND_RENDER_TARGET;
	}
	Image::Create(device, flags, width, height, nullptr, mipLevels);

	if (data)
	{
		Update(data, width);
	}

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {
		.Format        = format,
	    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D     = { .MostDetailedMip = 0, .MipLevels = mipLevels, }
    };

	Check(device->CreateShaderResourceView(*this, &viewDesc, &descriptor));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
	    .Format = format,
	    .ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
	    .Texture2D = {
	        .MipSlice = 0,
	    },
	};
	Check(device->CreateUnorderedAccessView(*this, &uavDesc, &uav));
}

void Texture::As(DescriptorBuffer *descriptorBuffer, size_t index)
{
	auto pDescriptor = descriptorBuffer->DeRef<uint64_t>(index);
	pDescriptor[0] = descriptor;
}

Texture::operator uint64_t() const
{
	return (uint64_t)descriptor;
}

bool Texture::operator==(const Super &_other) const
{
	const Texture &other = dynamic_cast<const Texture&>(_other);
	return handle == other.handle;
}

void Texture::Update(const void *data, uint32_t pitchX)
{
	auto context = device->GetContext();

	context->UpdateSubresource(*this, 0, nullptr, data, pitchX * format.Size(), 0);
}

void Texture::Blit()
{
	if (mipLevels > 1)
	{
		auto context = device->GetContext();
		context->GenerateMips(descriptor);
	}
}

}
}
