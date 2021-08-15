#include "impch.h"
#include "Queue.h"

#include "Device.h"

namespace Immortal
{
namespace Vulkan
{
	Queue::Queue(Device &device, UINT32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, UINT32 index) :
		device{ device },
		familyIndex{ familyIndex },
		index{ index },
		presented{ canPresent },
		properties{ properties }
	{
		vkGetDeviceQueue(device.Handle(), familyIndex, index, &handle);
	}

	Queue::Queue(Queue &&other) :
		device{ other.device },
		familyIndex{ other.familyIndex },
		index{ other.index },
		presented{ other.presented },
		properties{ properties }
	{

	}

	Queue::~Queue()
	{

	}
}
}
