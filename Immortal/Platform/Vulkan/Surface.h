#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class Instance;
class Surface
{
public:
    using Primitive = VkSurfaceKHR;
    VKCPP_OPERATOR_HANDLE()

public:
    Surface();

    ~Surface();

    VkResult Create(Instance *instance, Window *window, const VkAllocationCallbacks *pAllocator = nullptr);

    void Release(Instance *instance);

public:
    static VkResult CreateObject(VkInstance instance, Window::Type type, Anonymous window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator = nullptr);
};

}
}
