#pragma once

#include "vkcommon.h"

namespace Immortal
{
namespace Vulkan
{
	class CommandPool;

	class CommandBuffer
	{
	public:
		enum class ResetMode
		{
			ResetPool,
			ResetIndividually,
			AlwaysAllocated
		};

		enum class State
		{
			Invalid,
			Initial,
			Recording,
			Executable
		};

		VkResult reset(ResetMode reset_mode);

	private:
		CommandPool &commandPool;

		State state{ State::Initial };

		VkCommandBuffer handle{ VK_NULL_HANDLE };
	};
}
}
