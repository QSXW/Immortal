#pragma once

#include "Common.h"
#include "Handle.h"
#include "Algorithm/LightArray.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Framebuffer : public Handle<VkFramebuffer>
{
public:
	VKCPP_SWAPPABLE(Framebuffer)

public:
    Framebuffer();

    Framebuffer(Device *device, VkRenderPass renderPass, const LightArray<VkImageView> &views, const VkExtent2D &extent);

    ~Framebuffer();

    void Release();

public:
    void Swap(Framebuffer &other)
    {
		Handle::Swap(other);
		std::swap(device, other.device);
    }

private:
    Device *device{ nullptr };
};

}
}
