#pragma once

#include <map>

#include "VulkanCommon.h"
#include "ImmortalCore.h"

#include "VulkanInstance.h"
#include "VulkanQueue.h"

namespace Immortal
{
	/**
	  * @brief A physical Device usually represents a single complete implementation of Vulkan
	  * (excluding instance-level functionality) available to the host, of which there are a finite number.
	  *
	  */
	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice() = default;
		VulkanPhysicalDevice(VulkanInstance &instance, VkPhysicalDevice physicalDevice);

	private:
		VkPhysicalDevice mHandle{ VK_NULL_HANDLE };

	public:
		VulkanInstance &Instance;

		VkPhysicalDeviceFeatures Features;

		VkPhysicalDeviceProperties Properties;

		VkPhysicalDeviceMemoryProperties MemoryProperties;

		std::vector<VkQueueFamilyProperties> QueueFamilyProperties{};

		// The features that will be requested to be enabled in the logical device
		VkPhysicalDeviceFeatures RequestedFeatures{};

		void *LastRequestedExtensionFeature{ nullptr };
		
		std::map<VkStructureType, Ref<void>> ExtensionFeatures;

		bool HighPriorityGraphicsQueue{};
	};


	/**
	  * @brief Once Vulkan is initialized, devices and queues are the primary objects used
	  * to interact with a Vulkan implementation.
	  * 
	  * Vulkan Device, aka logical devices, represents an instance of that implementation with
	  * its own state and resources independent of other logical devices.
	  * Each device exposes a number of queue families each having one or more queues. All queues
	  * in a queue family support the same operations.
	  */
	class VulkanDevice
	{
	public:
		VulkanDevice() = default;
		VulkanDevice(VulkanPhysicalDevice &gpu, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requestedExtensions = {});
		~VulkanDevice();

		DefineGetHandleFunc(VkDevice)

		UINT32 QueueFailyIndex(VkQueueFlagBits queueFlag);

	private:
		const VulkanPhysicalDevice &mGraphicsProcessingUnit;

		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };

		VkDevice mHandle{ VK_NULL_HANDLE };

		std::vector<VkExtensionProperties> mDeviceExtensions;

		std::vector<const char *> mEnabledExtensions{};

		VmaAllocator mMemoryAllocator{ VK_NULL_HANDLE };

		std::vector<std::vector<VulkanQueue>> mQueues;

	};
}

