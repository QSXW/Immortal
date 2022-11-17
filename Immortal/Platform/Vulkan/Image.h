#pragma once

#include "Common.h"
#include "Core.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class ImageView;
class Image
{
public:
    using Primitive = VkImage;
    VKCPP_OPERATOR_HANDLE()

public:
    Image();

    Image(Device *device, VkImage handle, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags imageUsage, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

    Image(Device *device, const VkExtent3D &extent, VkFormat format,VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage,
          VkSampleCountFlagBits sampleCount   = VK_SAMPLE_COUNT_1_BIT,
          uint32_t mipLevels                  = 1,
          uint32_t arrayLayers                = 1,
          VkImageTiling      tiling           = VK_IMAGE_TILING_OPTIMAL,
          VkImageCreateFlags flags            = 0,
          uint32_t           numQueueFamilies = 0,
          const uint32_t    *queueFamilies   = nullptr);

    Image(Device *device, const VkImageCreateInfo &createInfo, VmaMemoryUsage memoryUsage);

    Image(Image &&other);

    ~Image();

    Image &operator=(Image &&other);

    void Invalidate(const VkImageCreateInfo &createInfo, VmaMemoryUsage memoryUsage);

    void Instantiate(VmaMemoryUsage memoryUsage);

    void Destroy();

    void Swap(Image &other);

public:
    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return device;
        }
    }

    const VkImageCreateInfo &GetProperties() const
    {
        return properties;
    }

    const VkExtent3D &GetExtent() const
    {
        return properties.extent;
    }

    VkImageType GetType() const
    {
        return properties.imageType;
    }

    uint32_t GetArrayLayer() const
    {
        return properties.arrayLayers;
    }

    uint32_t GetMipLevels() const
    {
        return properties.mipLevels;
    }

    VkFormat GetFormat() const
    {
        return properties.format;
    }

    const VkSampleCountFlagBits& SampleCount() const
    {
        return properties.samples;
    }

    const VkImageUsageFlags &Usage() const
    {
        return properties.usage;
    }

    const VkImageCreateInfo &Info() const
    {
        return properties;
    }

    bool IsDepth() const
    {
        return IsDepthOnlyFormat(properties.format);
    }

    void Map(void **dataMapped);

    void Unmap();

    Image(const Image &other) = delete;
    Image &operator=(const Image &other) = delete;

private:
    Device *device{ nullptr };

    VmaAllocation memory{ VK_NULL_HANDLE };

    VkImageCreateInfo properties{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
};

}

}
