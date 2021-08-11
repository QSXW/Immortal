#include "impch.h"
#include "VulkanPhysicalDevice.h"

namespace Immortal
{
	VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanInstance &instance, VkPhysicalDevice physicalDevice) :
		handle{ physicalDevice },
		Instance{ instance }
	{
		vkGetPhysicalDeviceFeatures(handle, &Features);
		vkGetPhysicalDeviceProperties(handle, &Properties);
		vkGetPhysicalDeviceMemoryProperties(handle, &MemoryProperties);

		IM_CORE_INFO("Found GPU: {0}", Properties.deviceName);

		UINT32 queueFamilyPropertiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyPropertiesCount, nullptr);
		QueueFamilyProperties.resize(queueFamilyPropertiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyPropertiesCount, QueueFamilyProperties.data());
	}

	VkBool32 VulkanPhysicalDevice::IsPresentSupported(VkSurfaceKHR surface, UINT32 queueFamilyIndex) NOEXCEPT
	{
		VkBool32 presentSupported{ VK_FALSE };

		Vulkan::Check(vkGetPhysicalDeviceSurfaceSupportKHR(handle, queueFamilyIndex, surface, &presentSupported));

		return presentSupported;
	}
}