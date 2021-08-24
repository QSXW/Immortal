#pragma once

#include "vkcommon.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan {
	class ImageView;
	class Image
	{
	public:
		Image(Device&               device,
			  VkImage               handle,
			  const VkExtent3D     &extent,
			  VkFormat              format,
			  VkImageUsageFlags     imageUsage,
			  VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

		Image(Device               &device,
			  const VkExtent3D     &extent,
			  VkFormat              format,
			  VkImageUsageFlags     imageUsage,
			  VmaMemoryUsage        memoryUsage,
			  VkSampleCountFlagBits sampleCount       = VK_SAMPLE_COUNT_1_BIT,
			  UINT32                mipLevels         = 1,
			  UINT32                arrayLayers       = 1,
			  VkImageTiling         tiling            = VK_IMAGE_TILING_OPTIMAL,
			  VkImageCreateFlags    flags             = 0,
			  UINT32                numQueueFamilies  = 0,
			  const UINT32*         queueFamilies     = nullptr);

		Image(Image&& other) noexcept;

	public:
		VkImage &Handle()
		{
			return handle;
		}

		template <class T>
		T& Get()
		{
			if constexpr (std::is_same_v<T, Device>)
			{
				return device;
			}
		}

		const VkExtent3D& Extent() const
		{
			return mExtent;
		}

		const VkImageType& Type() const
		{
			return mType;
		}

		const VkFormat& Format() const
		{
			return mFormat;
		}

		const VkSampleCountFlagBits& SampleCount() const
		{
			return mSampleCount;
		}

		const VkImageUsageFlags& Usage() const
		{
			return mUsage;
		}

		const VkImageSubresource& Subresource() const
		{
			return mSubresource;
		}

		std::unordered_set<ImageView*>& Views()
		{
			return mViews;
		}

	private:
		Device& device;

		VkImage handle{ VK_NULL_HANDLE };

		VmaAllocation mMemory{ VK_NULL_HANDLE };

		VkImageType mType{};

		VkExtent3D mExtent;

		VkFormat mFormat{};

		VkImageUsageFlags mUsage{};

		VkSampleCountFlagBits mSampleCount{};

		VkImageTiling mTiling{};

		VkImageSubresource mSubresource{};

		UINT32 mArrayLayerCount{ 0 };

		std::unordered_set<ImageView*> mViews;
		UINT8* mMappedData{ nullptr };
		bool mMapped{ false };
	};
}
}
