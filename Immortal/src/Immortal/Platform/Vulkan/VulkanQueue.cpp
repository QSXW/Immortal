#include "impch.h"
#include "VulkanQueue.h"

#include "VulkanDevice.h"

namespace Immortal
{
	VulkanQueue::VulkanQueue(VulkanDevice &device, UINT32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, UINT32 index) :
		mFamilyIndex{ familyIndex },
		mIndex{ index },
		mCanPresent{ canPresent },
		mProperties{ properties }
	{
		vkGetDeviceQueue(device.Handle(), familyIndex, index, &mHandle);
	}
}