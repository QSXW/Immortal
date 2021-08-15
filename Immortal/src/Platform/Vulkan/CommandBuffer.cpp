#pragma once

#include "vkcommon.h"
#include "CommandBuffer.h"
#include "CommandPool.h"

namespace Immortal
{
namespace Vulkan
{
	VkResult CommandBuffer::reset(ResetMode resetMode)
	{
		VkResult result = VK_SUCCESS;

		SLASSERT(resetMode == commandPool.Get<ResetMode>(),
			"The resetMode of Command buffer must match the one used by the pool to allocated it");
		state = State::Initial;
		if (resetMode == ResetMode::ResetIndividually)
		{
			result = vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		}

		return result;
	}
}
}
