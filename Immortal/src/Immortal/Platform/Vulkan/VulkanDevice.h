#pragma once

#include <map>

#include "VulkanCommon.h"
#include "ImmortalCore.h"

#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanQueue.h"

namespace Immortal
{
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

		void CheckExtensionSupported() NOEXCEPT;

		bool IsExtensionSupport(const char *extension) NOEXCEPT;

		bool IsEnabled(const char * extension) const NOEXCEPT;

		const VulkanQueue &FindQueueByFlags(VkQueueFlags flags, UINT32 queueIndex);
		const VulkanQueue &SuitableGraphicsQueue();

	//@inline
	public:
		NODISCARD VulkanPhysicalDevice &GraphicsProcessingUnit() NOEXCEPT
		{
			return mGraphicsProcessingUnit;
		}

		void WaitIdle() NOEXCEPT
		{
			vkDeviceWaitIdle(mHandle);
		}

	private:
		VulkanPhysicalDevice &mGraphicsProcessingUnit;

		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };

		VkDevice mHandle{ VK_NULL_HANDLE };

		std::vector<VkExtensionProperties> mDeviceExtensions;

		std::vector<const char *> mEnabledExtensions{};

		VmaAllocator mMemoryAllocator{ VK_NULL_HANDLE };

		std::vector<std::vector<VulkanQueue>> mQueues;

		bool mHasGetMemoryRequirements = false;
		bool mHasDedicatedAllocation = false;
		bool mHasBufferDeviceAddressName = false;

	};
}

