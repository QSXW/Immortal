#pragma once

#include "VulkanCommon.h"

namespace Immortal
{
	class VulkanRenderTarget
	{
	public:
		using CreateFunc = std::function<Unique<VulkanRenderTarget>(VulkanImage &&swapchainImage)>;
		static const CreateFunc DefaultCreateFunc;
	};
}

