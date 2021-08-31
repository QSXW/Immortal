#pragma once

#include "vkcommon.h"

namespace Immortal
{
namespace Vulkan
{
	class Device;

	class SemaphorePool
	{
	public:
		SemaphorePool(Device &device);
		
		~SemaphorePool();

		VkSemaphore Request();

		void Reset();

	private:
		Device &device;
		
		std::vector<VkSemaphore> handles;

		UINT32 activeCount{ 0 };
	};
}
}
