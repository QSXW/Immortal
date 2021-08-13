#include "impch.h"
#include "Queue.h"

#include "Device.h"

namespace Immortal
{
namespace Vulkan
{
	Queue::Queue(Device &device, UINT32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, UINT32 index) :
		device{ device },
		mFamilyIndex{ familyIndex },
		mIndex{ index },
		mCanPresent{ canPresent },
		mProperties{ properties }
	{
		vkGetDeviceQueue(device.Handle(), familyIndex, index, &handle);
	}

	Queue::Queue(Queue &&other) NOEXCEPT :
		device{ other.device },
		mFamilyIndex{ other.mFamilyIndex },
		mIndex{ other.mIndex },
		mCanPresent{ mCanPresent },
		mProperties{ mProperties }
	{

	}

	Queue::~Queue()
	{

	}
}
}