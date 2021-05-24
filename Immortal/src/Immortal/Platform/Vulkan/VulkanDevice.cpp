#include "impch.h"
#include "VulkanDevice.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

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

	VulkanDevice::VulkanDevice(VulkanPhysicalDevice &gpu, VkSurfaceKHR surface, std::unordered_map<const char*, bool> requestedExtensions)
		: mGraphicsProcessingUnit(gpu)
	{
		IM_CORE_INFO("Selected GPU: {0}", gpu.Properties.deviceName);

		UINT32 propsCount = static_cast<UINT32>(gpu.QueueFamilyProperties.size());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(propsCount, { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO });
		std::vector<std::vector<float>>      queueProps(propsCount);

		for (UINT32 index = 0; index < propsCount; index++)
		{
			const VkQueueFamilyProperties &prop = gpu.QueueFamilyProperties[index];
			if (gpu.HighPriorityGraphicsQueue)
			{
				UINT32 graphicsQueueFamily = QueueFailyIndex(VK_QUEUE_GRAPHICS_BIT);
				if (graphicsQueueFamily == index)
				{
					queueProps[index].reserve(prop.queueCount);
					queueProps[index].push_back(1.0f);
					for (UINT32 i = 1; i < prop.queueCount; i++)
					{
						queueProps[index].push_back(0.5f);
					}
				}
			}
			else
			{
				queueProps[index].resize(prop.queueCount, 0.5f);
			}

			auto &createInfo = queueCreateInfos[index];

			createInfo.pNext            = nullptr;
			createInfo.queueFamilyIndex = index; // This index corresponds to the index of an element of the pQueueFamilyProperties array that was returned by vkGetPhysicalDeviceQueueFamilyProperties
			createInfo.queueCount       = prop.queueCount; // must be greater than 0
			createInfo.pQueuePriorities = queueProps[index].data(); //e a valid pointer to an array of queueCount float value, must be between 0.0 and 1.0 inclusive
		}

	}

	UINT32 VulkanDevice::QueueFailyIndex(VkQueueFlagBits queueFlag)
	{
		const auto &queueFamilyProperties = mGraphicsProcessingUnit.QueueFamilyProperties;

		if (queueFlag & VK_QUEUE_COMPUTE_BIT)
		{
			for (UINT32 i = 0; i < static_cast<UINT32>(queueFamilyProperties.size()); i++)
			{
				if ((queueFamilyProperties[i].queueFlags & queueFlag) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
					break;
				}
			}
		}
	}


	/**
	 * @breif Queues are created along with a logical device during vkCreateDevice. All queues associated with a
     *  logical device are destroyed when vkDestroyDevice is called on that device
	 */
	VulkanDevice::~VulkanDevice()
	{
		static auto DestroyVmaAllocator = [](VmaAllocator memoryAllocator)
		{
			VmaStats stats;
			vmaCalculateStats(memoryAllocator, &stats);
			IM_CORE_INFO("Total device memory leaked: {0} bytes.", stats.total.usedBytes);

			vmaDestroyAllocator(memoryAllocator);
		};

		Vulkan::IfNotNullThen<VmaAllocator, DestroyVmaAllocator>(mMemoryAllocator);
		Vulkan::IfNotNullThen<VkDevice, &vkDestroyDevice>(mHandle);
	}
}
