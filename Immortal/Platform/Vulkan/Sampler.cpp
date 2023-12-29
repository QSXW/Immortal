#include "Sampler.h"
#include "Device.h"
#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

Sampler::Sampler() :
    Handle{},
    device{}
{

}

Sampler::Sampler(Device *device, Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod) :
    Handle{},
    device{ device }
{
   VkSamplerCreateInfo createInfo{
       .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
       .pNext            = nullptr,
       .magFilter        = VK_FILTER_NEAREST,
       .minFilter        = VK_FILTER_NEAREST,
       .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_NEAREST,
       .addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
       .addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
       .addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
       .mipLodBias       = 0,
	   .anisotropyEnable = VK_FALSE,
       .compareEnable    = compareOperation != CompareOperation::Never,
       .compareOp        = VkCompareOp(compareOperation),
       .minLod           = minLod,
       .maxLod           = maxLod,
       .borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
   };

    if (filter == Filter::Bilinear || filter == Filter::Linear)
    {
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.magFilter  = VK_FILTER_LINEAR;;
        createInfo.minFilter  = VK_FILTER_LINEAR;
    }
	if (false/*Anisotropic*/)
    {
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy = device->Get<PhysicalDevice>().Properties.limits.maxSamplerAnisotropy;
    }
	if (addressMode == AddressMode::Clamp || addressMode == AddressMode::BorderColor)
    {
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
    if (addressMode == AddressMode::BorderColor)
    {
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;		
    }
	if (addressMode == AddressMode::Mirror || addressMode == AddressMode::Repeat)
    {
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }

    Check(device->Create(&createInfo, &handle));
}

Sampler::Sampler(Device *device, const VkSamplerCreateInfo *pCreateInfo) :
    Handle{},
    device{ device }
{
    Check(device->Create(pCreateInfo, &handle));
}

Sampler::~Sampler()
{
	Release();
}

void Sampler::Release()
{
	if (device)
	{
		device->DestroyAsync(handle);
		handle = VK_NULL_HANDLE;
		device = nullptr;
	}
}

}
}
