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
    Check(device->Create(&info, nullptr, &handle));
}

Sampler::Sampler(Device *device, const Immortal::Texture::Description &desc) :
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

    if (desc.Filter == Texture::Filter::Bilinear)
    {
        createInfo.magFilter = VK_FILTER_LINEAR;;
        createInfo.minFilter = VK_FILTER_LINEAR;
    }
    if (desc.Anisotropic)
    {
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy = device->Get<PhysicalDevice>().Properties.limits.maxSamplerAnisotropy;
    }
    if (desc.Wrap == Texture::Wrap::Clamp)
    {
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    if (desc.Wrap == Texture::Wrap::Mirror)
    {
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
    if (desc.Wrap == Texture::Wrap::BorderColor)
    {
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    Check(device->Create(&createInfo, nullptr, &handle));
}

Sampler::~Sampler()
{
    if (!device)
    {
        return;
    }
    device->Destroy(handle);
}

Sampler::Sampler(Sampler &&other) :
    device{ other.device },
    handle{ other.handle }
{
    other.device = nullptr;
    other.handle = VK_NULL_HANDLE;
}

Sampler &Sampler::operator =(Sampler &&other)
{
    if (this == &other)
    {
        return *this;
    }

    device = other.device;
    handle = other.handle;

    other.device = nullptr;
    other.handle = VK_NULL_HANDLE;

    return *this;
}

}
}
