#include "impch.h"
#include "PhysicalDevice.h"

namespace Immortal
{
namespace Vulkan
{

PhysicalDevice::PhysicalDevice(Instance &instance, VkPhysicalDevice physicalDevice) :
    handle{ physicalDevice },
    instance{ instance }
{
    vkGetPhysicalDeviceFeatures(handle, &Features);
    vkGetPhysicalDeviceProperties(handle, &Properties);
    vkGetPhysicalDeviceMemoryProperties(handle, &MemoryProperties);

    UINT32 count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, nullptr);
    QueueFamilyProperties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, QueueFamilyProperties.data());
}

}
}
