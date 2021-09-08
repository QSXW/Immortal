#pragma once

#include "vkcommon.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

class ImageView;
class Image
{
public:
    Image(Device               *device,
          VkImage               handle,
          const VkExtent3D     &extent,
          VkFormat              format,
          VkImageUsageFlags     imageUsage,
          VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

    Image(Device               *device,
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

    Image(Image &&other);

public:
    VkImage &Handle()
    {
        return handle;
    }

    template <class T>
    T &Get()
    {
        if constexpr (std::is_same_v<T, Device>)
        {
            return *device;
        }
    }

    const VkExtent3D& Extent() const
    {
        return extent;
    }

    const VkImageType& Type() const
    {
        return type;
    }

    const VkFormat& Format() const
    {
        return format;
    }

    const VkSampleCountFlagBits& SampleCount() const
    {
        return sampleCount;
    }

    const VkImageUsageFlags& Usage() const
    {
        return usage;
    }

    const VkImageSubresource& Subresource() const
    {
        return subresource;
    }

    std::unordered_set<ImageView*>& Views()
    {
        return views;
    }

private:
    Device *device;

    VkImage handle{ VK_NULL_HANDLE };

    VmaAllocation memory{ VK_NULL_HANDLE };

    VkImageType type{};

    VkExtent3D extent;

    VkFormat format{};

    VkImageUsageFlags usage{};

    VkSampleCountFlagBits sampleCount{};

    VkImageTiling tiling{};

    VkImageSubresource subresource{};

    UINT32 arrayLayerCount{ 0 };

    std::unordered_set<ImageView*> views;

    UINT8* mappedData{ nullptr };

    bool mapped{ false };
};
}
}
