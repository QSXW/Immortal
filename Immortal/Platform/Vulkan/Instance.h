#pragma once

#include "Core.h"

#include "Common.h"

namespace Immortal
{
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
             uint32_t                                     apiVersion               = VK_API_VERSION_1_0);

    Instance(VkInstance instance);

    ~Instance();

    PhysicalDevice &SuitablePhysicalDevice();

    void QueryPhysicalDevice();

    bool CheckValidationLayerSupport();

public:
    bool IsEnabled(const char *extension) const
    {
        return std::find_if(enabledExtensions.begin(), enabledExtensions.end(),
        [extension](const char *enabledExtension)
        {
            return Equals(extension, enabledExtension);
        }) != enabledExtensions.end();
    }

    VkInstance &Handle()
    {
        return handle;
    }

    operator VkInstance&()
    {
        return handle;
    }

    operator VkInstance() const
    {
        return handle;
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
