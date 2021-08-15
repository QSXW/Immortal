#pragma once

#include <map>

#include "vkcommon.h"
#include "ImmortalCore.h"

#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "CommandPool.h"

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

	public:
		Device() = default;
		Device(PhysicalDevice& physicalDevice, VkSurfaceKHR surface, std::unordered_map<const char*, bool> requestedExtensions = {});
		~Device();

		UINT32 QueueFailyIndex(VkQueueFlagBits queueFlag);

		void CheckExtensionSupported() NOEXCEPT;

		bool IsExtensionSupport(const char* extension) NOEXCEPT;

		bool IsEnabled(const char* extension) const NOEXCEPT;

		Queue& FindQueueByFlags(VkQueueFlags flags, UINT32 queueIndex);
		const Queue& SuitableGraphicsQueue();
		//@inline
	public:
		VkDevice &Handle()
		{
			return handle;
		}

		template <class T>
		T& Get()
		{
			if constexpr (std::is_same_v<T, VkDevice>)
			{
				return handle;
			}
			if constexpr (std::is_same_v<T, PhysicalDevice>)
			{
				return physicalDevice;
			}
			if constexpr (std::is_same_v<T, Queue>)
			{
				return queues;
			}
		}

		void WaitIdle()
		{
			vkDeviceWaitIdle(handle);
		}

		VmaAllocator MemoryAllocator() const
		{
			return mMemoryAllocator;
		}

	private:
		PhysicalDevice &physicalDevice;

		VkSurfaceKHR surface{ VK_NULL_HANDLE };

		VkDevice handle{ VK_NULL_HANDLE };

		std::vector<VkExtensionProperties> mDeviceExtensions;

		std::vector<const char*> enabledExtensions{};

		VmaAllocator mMemoryAllocator{ VK_NULL_HANDLE };

		std::vector<std::vector<Queue>> queues;
		Unique<CommandPool> commandPool;

		bool mHasGetMemoryRequirements = false;
		bool mHasDedicatedAllocation = false;
		bool mHasBufferDeviceAddressName = false;
	};
}
}

