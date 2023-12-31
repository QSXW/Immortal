#pragma once

#include "Common.h"
#include "Core.h"
#include "Handle.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class ImageView;
class Image : public Handle<VkImage>
{
public:
	VKCPP_SWAPPABLE(Image)

public:
    Image(Device *device = nullptr);

    Image(Device *device, VkImage handle, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags imageUsage, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

    Image(Device                           *device,
          const VkExtent3D                 &extent,
          VkFormat                          format,
          VkImageUsageFlags                 imageUsage,
          VmaMemoryUsage                    memoryUsage,
          VkSampleCountFlagBits             sampleCount      = VK_SAMPLE_COUNT_1_BIT,
          uint32_t                          mipLevels        = 1,
          uint32_t                          arrayLayers      = 1,
          VkImageTiling                     tiling           = VK_IMAGE_TILING_OPTIMAL,
          VkImageCreateFlags                flags            = 0,
          uint32_t                          numQueueFamilies = 0,
          const uint32_t                   *queueFamilies    = nullptr);

    Image(Device *device, const VkImageCreateInfo &createInfo, VmaMemoryUsage memoryUsage);

    ~Image();

    void Invalidate(const VkImageCreateInfo &createInfo, VmaMemoryUsage memoryUsage);

    void Instantiate(VmaMemoryUsage memoryUsage);

    void Release();

    void Map(void **ppData);

    void Unmap();

public:
    template <class T>
    T *Get()
    {
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return device;
        }
    }

    Image *GetImage()
    {
		return this;
    }

    const VkImageCreateInfo &GetProperties() const
    {
        return properties;
    }

    const VkExtent3D &GetExtent() const
    {
        return properties.extent;
    }

    VkImageCreateFlags GetFlags() const
    {
		return properties.flags;
    }

    VkImageType GetType() const
    {
        return properties.imageType;
    }

    uint32_t GetArrayLayers() const
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

    VkSampleCountFlagBits GetSamples() const
    {
        return properties.samples;
    }

    const VkImageUsageFlags &GetUsage() const
    {
        return properties.usage;
    }

    const VkImageCreateInfo &GetInfo() const
    {
        return properties;
    }

    bool IsDepth() const
    {
        return IsDepthOnlyFormat(properties.format);
    }
   
    void Swap(Image &other)
    {
		Handle::Swap(other);
		std::swap(device,     other.device    );
		std::swap(memory,     other.memory    );
		std::swap(properties, other.properties);
    }

protected:
    Device *device{ nullptr };

    VmaAllocation memory{ VK_NULL_HANDLE };

    VkImageCreateInfo properties{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
};

}

}
