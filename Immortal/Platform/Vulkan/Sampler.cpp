#include "Sampler.h"
#include "Device.h"
#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

Sampler::Sampler(Device *device, const VkSamplerCreateInfo &info) :
    device{ device }
{
    Check(device->Create(&info, &handle));
}

Sampler::Sampler(Device *device, const Immortal::Texture::Description &desc, uint32_t mipLevels) :
    device{ device }
{
    VkSamplerCreateInfo createInfo{};
    createInfo.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext         = nullptr;
    createInfo.magFilter     = VK_FILTER_NEAREST;
    createInfo.minFilter     = VK_FILTER_NEAREST;
    createInfo.addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    createInfo.minLod        = 0;
    createInfo.maxLod        = mipLevels;
    createInfo.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    if (desc.filter == Filter::Bilinear || desc.filter == Filter::Linear)
    {
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;;
        createInfo.minFilter = VK_FILTER_LINEAR;
    }
    if (desc.Anisotropic)
    {
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy = device->Get<PhysicalDevice>().Properties.limits.maxSamplerAnisotropy;
    }
    if (desc.wrap == Wrap::Clamp || desc.wrap == Wrap::BorderColor)
    {
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
    if (desc.wrap == Wrap::Mirror || desc.wrap == Wrap::Repeat)
    {
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }

    Check(device->Create(&createInfo, &handle));
}

Sampler::~Sampler()
{
    if (device)
    {
        device->DestroyAsync(handle);
    }
}

Sampler::Sampler(Sampler &&other) :
    Sampler{}
{
    other.Swap(*this);
}

Sampler &Sampler::operator =(Sampler &&other)
{
    Sampler(std::move(other)).Swap(*this);
    return *this;
}

void Sampler::Swap(Sampler &other)
{
    std::swap(device, other.device);
    std::swap(handle, other.handle);
}

}
}
