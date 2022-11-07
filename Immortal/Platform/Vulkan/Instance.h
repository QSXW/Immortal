#pragma once

#include "Core.h"
#include "Common.h"
#include "Interface/IObject.h"

namespace Immortal
{

class Window;
namespace Vulkan
{

class PhysicalDevice;
class Instance
{
public:
    using Primitive = VkInstance;
    VKCPP_OPERATOR_HANDLE()

public:
    void Destroy(VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyInstance(handle, pAllocator);
    }

    VkResult EnumeratePhysicalDevices(uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices)
    {
        return vkEnumeratePhysicalDevices(handle, pPhysicalDeviceCount, pPhysicalDevices);
    }

    PFN_vkVoidFunction GetProcAddr(char const *pName)
    {
        return vkGetInstanceProcAddr(handle, pName);
    }

    VkResult CreateDisplayPlaneSurfaceKHR(VkDisplaySurfaceCreateInfoKHR const *pCreateInfo, VkSurfaceKHR *pSurface, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDisplayPlaneSurfaceKHR(handle, pCreateInfo, pAllocator, pSurface);
    }

    void DestroySurfaceKHR(VkSurfaceKHR surface, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroySurfaceKHR(handle, surface, pAllocator);
    }

    VkResult CreateDebugReportCallbackEXT(VkDebugReportCallbackCreateInfoEXT const *pCreateInfo, VkDebugReportCallbackEXT *pCallback, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDebugReportCallbackEXT(handle, pCreateInfo, pAllocator, pCallback);
    }

    void DestroyDebugReportCallbackEXT(VkDebugReportCallbackEXT callback, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyDebugReportCallbackEXT(handle, callback, pAllocator);
    }

    void DebugReportMessageEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, char const *pLayerPrefix, char const *pMessage)
    {
        vkDebugReportMessageEXT(handle, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    }

    VkResult EnumeratePhysicalGroups(uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties)
    {
        return vkEnumeratePhysicalDeviceGroups(handle, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    }

    VkResult CreateDebugUtilsMessengerEXT(VkDebugUtilsMessengerCreateInfoEXT const *pCreateInfo, VkDebugUtilsMessengerEXT *pMessenger, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDebugUtilsMessengerEXT(handle, pCreateInfo, pAllocator, pMessenger);
    }

    void DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyDebugUtilsMessengerEXT(handle, messenger, pAllocator);
    }

    void SubmitDebugUtilsMessageEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData)
    {
        vkSubmitDebugUtilsMessageEXT(handle, messageSeverity, messageTypes, pCallbackData);
    }

    VkResult CreateHeadlessSurfaceEXT(VkHeadlessSurfaceCreateInfoEXT const *pCreateInfo, VkSurfaceKHR *pSurface, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateHeadlessSurfaceEXT(handle, pCreateInfo, pAllocator, pSurface);
    }

#ifdef _WIN32
    VkResult CreateWin32SurfaceKHR(VkWin32SurfaceCreateInfoKHR const *pCreateInfo, VkSurfaceKHR *pSurface, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateWin32SurfaceKHR(handle, pCreateInfo, pAllocator, pSurface);
    }
#endif

#ifdef __APPLE__
    VkResult CreateIOSSurfaceMVK(VkIOSSurfaceCreateInfoMVK const *pCreateInfo, VkSurfaceKHR *pSurface, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateIOSSurfaceMVK(handle, pCreateInfo, pAllocator, pSurface);
    }

    VkResult CreateMacOSSurfaceMVK(VkMacOSSurfaceCreateInfoMVK const *pCreateInfo, VkSurfaceKHR *pSurface, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateMacOSSurfaceMVK(handle, pCreateInfo, pAllocator, pSurface);
    }
#endif

public:
    Instance() = default;

    Instance(const char*                                  applicationName,
             const std::unordered_map<const char*, bool> &requiredExtension        = {},
             const std::vector<const char*>&              requiredValidationLayers = {},
             bool                                         headless                 = false,
             uint32_t                                     apiVersion               = VK_API_VERSION_1_2);

    Instance(VkInstance instance);

    ~Instance();

    PhysicalDevice *SuitablePhysicalDevice(int deviceId = AUTO_DEVICE_ID);

    void QueryPhysicalDevice();

public:
    VkResult CreateSurface(Window *window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator = nullptr);

    static VkResult CreateSurface(VkInstance instance, Anonymous window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator = nullptr);

public:
    bool IsEnabled(const char *extension) const
    {
        return std::find_if(enabledExtensions.begin(), enabledExtensions.end(),
        [extension](const char *enabledExtension)
        {
            return Equals(extension, enabledExtension);
        }) != enabledExtensions.end();
    }

    bool Ready() const
    {
        return !!handle;
    }

private:
    std::vector<const char*> enabledExtensions;

    std::vector<URef<PhysicalDevice>> physicalDevices;

#if defined (_DEBUG) || defined (VKB_VALIDATION_LAYERS)
    VkDebugUtilsMessengerEXT debugUtilsMessengers{ VK_NULL_HANDLE };

    VkDebugReportCallbackEXT debugReportCallback{ VK_NULL_HANDLE };
#endif
};

}
}
