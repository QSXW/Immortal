#include "impch.h"
#include "VulkanQueue.h"

#include "VulkanDevice.h"

namespace Immortal
{
	VulkanQueue::VulkanQueue(VulkanDevice &device, UINT32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, UINT32 index) :
		mDevice{ device },
		mFamilyIndex{ familyIndex },
		mIndex{ index },
		mCanPresent{ canPresent },
		mProperties{ properties }
	{
		vkGetDeviceQueue(device.Handle(), familyIndex, index, &mHandle);
	}

	VulkanQueue::VulkanQueue(VulkanQueue && other) NOEXCEPT :
		mDevice{ other.mDevice },
		mFamilyIndex{ other.mFamilyIndex },
		mIndex{ other.mIndex },
		mCanPresent{ mCanPresent },
		mProperties{ mProperties }
	{

	}

	VulkanQueue::~VulkanQueue()
	{
		
	}
}