#include "impch.h"
#include "VulkanDevice.h"

namespace Immortal
{
	VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice physicalDevice)
		: mHandle{ physicalDevice }
	{
		vkGetPhysicalDeviceFeatures(mHandle, &mFeatures);
		vkGetPhysicalDeviceProperties(mHandle, &mProperties);
		vkGetPhysicalDeviceMemoryProperties(mHandle, &mMemoryProperties);

		IM_CORE_INFO("Found GPU: {0}", mProperties.deviceName);

		UINT32 queueFamilyPropertiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(mHandle, &queueFamilyPropertiesCount, nullptr);
		mQueueFamilyProperties.reserve(queueFamilyPropertiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(mHandle, &queueFamilyPropertiesCount, mQueueFamilyProperties.data());
	}

	VulkanDevice::VulkanDevice(VulkanPhysicalDevice & gpu, VkSurfaceKHR surface, std::unordered_map<const char*, bool> requestedExtensions)
	{

	}


}
