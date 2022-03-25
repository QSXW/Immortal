#pragma once

#include "Core.h"
#include "Common.h"

namespace Immortal
{

class Window;
namespace Vulkan
{

class PhysicalDevice;
class Instance
{
public:
    Instance() = default;

    Instance(const char*                                  applicationName,
             const std::unordered_map<const char*, bool> &requiredExtension        = {},
             const std::vector<const char*>&              requiredValidationLayers = {},
             bool                                         headless                 = false,
             uint32_t                                     apiVersion               = VK_API_VERSION_1_2);

    Instance(VkInstance instance);

    ~Instance();

    PhysicalDevice &SuitablePhysicalDevice();

    void QueryPhysicalDevice();

public: /* vk api */
    VkResult EnumeratePhysicalDevices(uint32_t *physicalDeviceCount, VkPhysicalDevice *pPhysicalDevice) const
    {
        return vkEnumeratePhysicalDevices(handle, physicalDeviceCount, pPhysicalDevice);
    }

    template <class T>
    T GetProcAddr(const std::string &func) const
    {
        return (T)vkGetInstanceProcAddr(handle, func.c_str());
    }

    void DestroySurface(VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator = nullptr) const
    {
        vkDestroySurfaceKHR(handle, surface, pAllocator);
    }

    VkResult CreateSurface(Window *window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator = nullptr) const;

public:
    bool IsEnabled(const char *extension) const
    {
        return std::find_if(enabledExtensions.begin(), enabledExtensions.end(),
        [extension](const char *enabledExtension)
        {
            return Equals(extension, enabledExtension);
        }) != enabledExtensions.end();
    }

    VkInstance Handle() const
    {
        return handle;
    }

    operator VkInstance() const
    {
        return handle;
    }

    bool Ready() const
    {
        return !!handle;
    }

private:
    VkInstance handle{ VK_NULL_HANDLE };

    std::vector<const char*> enabledExtensions{};

    std::vector<std::unique_ptr<PhysicalDevice>> physicalDevices{};

#if defined ( SLDEBUG ) || defined ( VKB_VALIDATION_LAYERS )
    VkDebugUtilsMessengerEXT debugUtilsMessengers{ VK_NULL_HANDLE };

    VkDebugReportCallbackEXT debugReportCallback{ VK_NULL_HANDLE };
#endif
};

}
}
