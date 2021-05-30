#include "impch.h"
#include "VulkanRenderTarget.h"

#include "VulkanImage.h"

namespace Immortal
{
	const VulkanRenderTarget::CreateFunc VulkanRenderTarget::DefaultCreateFunc = [](VulkanImage &&swapchainImage) -> Unique<VulkanRenderTarget>
	{
		return MakeUnique<VulkanRenderTarget>();
	};
}