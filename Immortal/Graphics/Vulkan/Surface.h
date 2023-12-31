#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class Instance;
class Surface : public Handle<VkSurfaceKHR>
{
public:
	VKCPP_SWAPPABLE(Surface)

public:
    Surface();

    ~Surface();

    VkResult Create(Instance *instance, Window *window, const VkAllocationCallbacks *pAllocator = nullptr);

    void Release(Instance *instance);

public:
	void Swap(Surface &other)
    {
		Handle::Swap(other);
    }

public:
    static VkResult CreateObject(VkInstance instance, Window::Type type, Anonymous window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator = nullptr);
};

}
}
