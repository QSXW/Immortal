#pragma once

#include "VulkanCommon.h"

namespace Immortal
{
	class VulkanDevice;
	class VulkanQueue
	{
		VulkanQueue(VulkanDevice &device, UINT32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, UINT32 index);
		VulkanQueue(const VulkanQueue &) = default;
		VulkanQueue(VulkanQueue && other);

		DefineGetHandleFunc(VkQueue)

	private:
		VulkanDevice *device;

		VkQueue mHandle{ VK_NULL_HANDLE };

		UINT32 mFamilyIndex = 0;

		UINT32 mIndex = 0;

		VkBool32 mCanPresent{ VK_FALSE };

		VkQueueFamilyProperties mProperties{};

	private:
		VulkanQueue &operator=(const VulkanQueue &) = delete;
		VulkanQueue &operator=(VulkanQueue &&) = delete;
	};
}

