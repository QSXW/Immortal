#pragma once

#include "Common.h"
#include <queue>
#include <mutex>

namespace Immortal
{
namespace Vulkan
{
class Device;

class FencePool
{
public:
	using iterator                    = std::vector<VkFence>::iterator;
	static constexpr uint64_t Timeout = std::numeric_limits<uint64_t>::max();

public:
	FencePool(Device *device);
		
	~FencePool();

	VkResult Wait(uint32_t timeout = std::numeric_limits<uint32_t>::max()) const;

	VkResult Reset();

	VkFence Request();

	VkFence &operator[](size_t index)
	{
		return handles[index];
	}

	void Discard(VkFence *pFence);

private:
	Device *device{ nullptr };

	std::vector<VkFence> handles;

	std::queue<VkFence> pending;

	uint32_t activeCount{ 0 };

	std::mutex mutux;
};

}
}
