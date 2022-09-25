#pragma once

#include "Render/Texture.h"
#include "Common.h"
#include "Image.h"
#include "Descriptor.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class Texture : public SuperTexture, public Image
{
public:
    using Super = SuperTexture;

public:
	Texture(Device *device, const std::string &filepath, const Description &description);

    Texture(Device *device, uint32_t width, uint32_t height, const void *data, const Description &description);

    ~Texture();

    virtual operator uint64_t() const override;

    virtual bool operator==(const Super &other) const override;

    virtual void As(DescriptorBuffer *descriptorBuffer, size_t index) override;

    virtual void Update(const void *data, uint32_t pitchX = 0) override;

    virtual void Blit() override;

    const Descriptor<SRV> GetDescriptor() const
    {
		return descriptor;
    }

    const Descriptor<UAV> GetUAV() const
    {
		return uav;
    }

protected:
    void __Create(const void *data);

protected:
	Device *device;

    Descriptor<SRV> descriptor;

    Descriptor<UAV> uav;
};

}
}
