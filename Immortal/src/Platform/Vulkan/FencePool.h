#pragma once

#include "vkcommon.h"

namespace Immortal
{
namespace Vulkan
{
	class Device;

	class FencePool
	{
	public:
		FencePool(Device &device);
		
		~FencePool();

		VkResult Wait(UINT32 timeout = std::numeric_limits<UINT32>::max()) const;

		VkResult Reset();

		VkFence RequestFence();

	private:
		Device &device;
		
		std::vector<VkFence> handles;

		UINT32 activeCount{ 0 };
	};
}
}
