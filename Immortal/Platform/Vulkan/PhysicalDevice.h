#pragma once

#include "Core.h"
#include "Common.h"

#include "Instance.h"

namespace Immortal
{
namespace Vulkan
{
class PhysicalDevice
{
public:
    PhysicalDevice() = default;
    PhysicalDevice(Instance &instance, VkPhysicalDevice physicalDevice);

    template <typename T>
    T &RequestExtensionFeatures(VkStructureType type)
    {
        // We cannot request extension features if the physical device properties 2 instance extension isnt enabled
        if (!instance.IsEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
        {
            LOG::ERR("Couldn't request feature from device as " + std::string(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) + " isn't enabled!");
        }

        // If the type already exists in the map, return a casted pointer to get the extension feature struct
        auto extensionFeaturesIterator = ExtensionFeatures.find(type);
        if (extensionFeaturesIterator != ExtensionFeatures.end())
        {
            return *static_cast<T *>(extensionFeaturesIterator->second.get());
        }

        // Get the extension feature
        VkPhysicalDeviceFeatures2KHR physicalDeviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
        T extension{ type };
        physicalDeviceFeatures.pNext = &extension;

        auto vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance.Handle(), "vkGetPhysicalDeviceFeatures2KHR");

        vkGetPhysicalDeviceFeatures2KHR(handle, &physicalDeviceFeatures);

        // Insert the extension feature into the extension feature map so its ownership is held
        ExtensionFeatures.insert({ type, std::make_shared<T>(extension) });

        // Pull out the dereferenced void pointer, we can assume its type based on the template
        auto *pExtension = static_cast<T *>(ExtensionFeatures.find(type)->second.get());

        // If an extension feature has already been requested, we shift the linked list down by one
        // Making this current extension the new base pointer
        if (LastRequestedExtensionFeature)
        {
            pExtension->pNext = LastRequestedExtensionFeature;
        }
        LastRequestedExtensionFeature = pExtension;

        return *pExtension;
    }

public:
    VkPhysicalDevice &Handle()
    {
        return handle;
    }

    operator VkPhysicalDevice&()
    {
        return handle;
    }

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<Instance, T>())
        {
            return instance;
        }
    }

    VkFormat DepthFormat(bool depthOnly = false)
    {
        return SuitableDepthFormat(handle, depthOnly);
    }

    VkBool32 IsPresentSupported(VkSurfaceKHR surface, UINT32 queueFamilyIndex)
    {
        VkBool32 presentSupported{ VK_FALSE };
        Check(vkGetPhysicalDeviceSurfaceSupportKHR(handle, queueFamilyIndex, surface, &presentSupported));
        return presentSupported;
    }

private:
    VkPhysicalDevice handle{ VK_NULL_HANDLE };

public:
    Instance &instance;

    VkPhysicalDeviceFeatures Features;

    VkPhysicalDeviceProperties Properties;

    VkPhysicalDeviceMemoryProperties MemoryProperties;

    std::vector<VkQueueFamilyProperties> QueueFamilyProperties{};

    std::map<VkStructureType, std::shared_ptr<void>> ExtensionFeatures;

    // The features that will be requested to be enabled in the logical device
    VkPhysicalDeviceFeatures RequestedFeatures{};

    void *LastRequestedExtensionFeature{ nullptr };

    bool HighPriorityGraphicsQueue{};
};
}
}
