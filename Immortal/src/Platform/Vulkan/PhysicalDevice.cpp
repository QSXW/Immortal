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

		Log::Info("Found GPU: {0}", Properties.deviceName);

		UINT32 queueFamilyPropertiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyPropertiesCount, nullptr);
		QueueFamilyProperties.resize(queueFamilyPropertiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyPropertiesCount, QueueFamilyProperties.data());
	}
}
}
