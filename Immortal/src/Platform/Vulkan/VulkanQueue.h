#pragma once

#include "VulkanCommon.h"

namespace Immortal
{
	class VulkanDevice;
	class VulkanQueue
	{
	public:
		enum class Type
		{
			Graphics      = VK_QUEUE_GRAPHICS_BIT,
			Compute       = VK_QUEUE_COMPUTE_BIT,
			Transfer      = VK_QUEUE_TRANSFER_BIT,
			SparseBinding = VK_QUEUE_SPARSE_BINDING_BIT,
			Protected     = VK_QUEUE_PROTECTED_BIT,

#ifdef VK_ENABLE_BETA_EXTENSIONS
			VideoDeCode   = VK_QUEUE_VIDEO_DECODE_BIT_KHR,
			VideoEncode   =VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
#endif
			None
		};

	public:
		VulkanQueue(VulkanDevice &device, UINT32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, UINT32 index);
		VulkanQueue(const VulkanQueue &) = default;
		VulkanQueue(VulkanQueue && other) NOEXCEPT;
		
		~VulkanQueue();

	// @inline
	public:
		VkQueueFamilyProperties Properties() const NOEXCEPT
		{
			return mProperties;
		}

		VkBool32 IsPresentSupported() const NOEXCEPT
		{
			return mCanPresent;
		}

	//@predefine
	public:
		DefineGetHandleFunc(VkQueue)
	
	//@private
	private:
		VulkanDevice &mDevice;

		VkQueue handle{ VK_NULL_HANDLE };

		UINT32 mFamilyIndex = 0;

		UINT32 mIndex = 0;

		VkBool32 mCanPresent{ VK_FALSE };

		VkQueueFamilyProperties mProperties{};

	//@deleted
	private:
		//VulkanQueue &operator=(const VulkanQueue &) = delete;
		VulkanQueue &operator=(VulkanQueue &&) = delete;
	};
}

