#pragma once

#include <map>

#include "vkcommon.h"
#include "ImmortalCore.h"

#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"

namespace Immortal
{
namespace Vulkan
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
	class Device
	{
	public:
		Device() = default;
		Device(PhysicalDevice& gpu, VkSurfaceKHR surface, std::unordered_map<const char*, bool> requestedExtensions = {});
		~Device();

		DefineGetHandleFunc(VkDevice)

			UINT32 QueueFailyIndex(VkQueueFlagBits queueFlag);

		void CheckExtensionSupported() NOEXCEPT;

		bool IsExtensionSupport(const char* extension) NOEXCEPT;

		bool IsEnabled(const char* extension) const NOEXCEPT;

		const Queue& FindQueueByFlags(VkQueueFlags flags, UINT32 queueIndex);
		const Queue& SuitableGraphicsQueue();

		//@inline
	public:
		NODISCARD PhysicalDevice& GraphicsProcessingUnit() NOEXCEPT
		{
			return mGraphicsProcessingUnit;
		}

		void WaitIdle() NOEXCEPT
		{
			vkDeviceWaitIdle(handle);
		}

		VmaAllocator MemoryAllocator() const
		{
			return mMemoryAllocator;
		}

	private:
		PhysicalDevice& mGraphicsProcessingUnit;

		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };

		VkDevice handle{ VK_NULL_HANDLE };

		std::vector<VkExtensionProperties> mDeviceExtensions;

		std::vector<const char*> mEnabledExtensions{};

		VmaAllocator mMemoryAllocator{ VK_NULL_HANDLE };

		std::vector<std::vector<Queue>> mQueues;

		bool mHasGetMemoryRequirements = false;
		bool mHasDedicatedAllocation = false;
		bool mHasBufferDeviceAddressName = false;

	};
}
}

