#include "impch.h"
#include "VulkanPhysicalDevice.h"

namespace Immortal
{
	VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanInstance &instance, VkPhysicalDevice physicalDevice) :
		mHandle{ physicalDevice },
		Instance{ instance }
	{
		vkGetPhysicalDeviceFeatures(mHandle, &Features);
		vkGetPhysicalDeviceProperties(mHandle, &Properties);
		vkGetPhysicalDeviceMemoryProperties(mHandle, &MemoryProperties);

		IM_CORE_INFO("Found GPU: {0}", Properties.deviceName);

		UINT32 queueFamilyPropertiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mHandle, &queueFamilyPropertiesCount, nullptr);
		QueueFamilyProperties.resize(queueFamilyPropertiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(mHandle, &queueFamilyPropertiesCount, QueueFamilyProperties.data());
	}

	VkBool32 VulkanPhysicalDevice::IsPresentSupported(VkSurfaceKHR surface, UINT32 queueFamilyIndex) NOEXCEPT
	{
		VkBool32 presentSupported{ VK_FALSE };

		Vulkan::Check(vkGetPhysicalDeviceSurfaceSupportKHR(mHandle, queueFamilyIndex, surface, &presentSupported));

		return presentSupported;
	}
}