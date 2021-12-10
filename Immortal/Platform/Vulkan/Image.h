#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
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

    Image(Device            *device, 
          VkImageCreateInfo &createInfo,
          VmaMemoryUsage     memoryUsage);

    Image(Image &&other);

    ~Image();

    Image &operator=(Image &&other);

public:
    VkImage &Handle()
    {
        return handle;
    }

    operator VkImage&()
    {
        return handle;
    }

    operator const VkImage&() const
    {
        return handle;
    }

    template <class T>
    T Get()
    {
        if constexpr (std::is_same_v<T, Device *>)
        {
            return device;
        }
    }

    const VkExtent3D &Extent() const
    {
        return info.extent;
    }

    const VkImageType &Type() const
    {
        return info.imageType;
    }

    const VkFormat& Format() const
    {
        return info.format;
    }

    const VkSampleCountFlagBits& SampleCount() const
    {
        return info.samples;
    }

    const VkImageUsageFlags& Usage() const
    {
        return info.usage;
    }

    const VkImageSubresource &Subresource() const
    {
        return subresource;
    }

    std::unordered_set<ImageView *> &Views()
    {
        return views;
    }

    bool IsDepth() const
    {
        return IsDepthOnlyFormat(info.format);
    }

private:
    Device *device{ nullptr };

    VkImage handle{ VK_NULL_HANDLE };

    VmaAllocation memory{ VK_NULL_HANDLE };

    VkImageCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

    VkImageSubresource subresource;

    std::unordered_set<ImageView*> views;

    UINT8* mappedData{ nullptr };

    bool mapped{ false };
};
}

}
