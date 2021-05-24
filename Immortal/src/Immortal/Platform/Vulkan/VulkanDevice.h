#pragma once

#include <map>

#include "VulkanCommon.h"
#include "ImmortalCore.h"

namespace Immortal
{
	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice() = default;
		VulkanPhysicalDevice(VkPhysicalDevice physicalDevice);

		const VkPhysicalDeviceProperties &Properties() const NOEXCEPT
		{
			return mProperties;
		}

		VkPhysicalDeviceFeatures &Features() NOEXCEPT
		{
			return mFeatures;
		}

	private:
		VkPhysicalDevice mHandle{ VK_NULL_HANDLE };

		VkPhysicalDeviceFeatures mFeatures;

		VkPhysicalDeviceProperties mProperties;

		VkPhysicalDeviceMemoryProperties mMemoryProperties;

		std::vector<VkQueueFamilyProperties> mQueueFamilyProperties{};

		// The features that will be requested to be enabled in the logical device
		VkPhysicalDeviceFeatures mRequestedFeatures{};

		void *mLastRequestedExtensionFeature{ nullptr };
		
		std::map<VkStructureType, Ref<void>> mExtensionFeatures;

		bool mHighPriorityGraphicsQueue{};
	};

	class VulkanDevice
	{
	public:
		VulkanDevice() = default;
		VulkanDevice(VulkanPhysicalDevice &gpu, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requestedExtensions = {});

	};
}

