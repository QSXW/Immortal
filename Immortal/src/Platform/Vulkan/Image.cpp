#include "impch.h"
#include "Image.h"

#include "ImageView.h"

namespace Immortal
{
namespace Vulkan
{
inline VkImageType ImageType(VkExtent3D extent)
{
    VkImageType result{};

    uint32_t depth{ 0 };

    if (extent.width >= 1)
    {
        depth++;
    }

    if (extent.height >= 1)
    {
        depth++;
    }

    if (extent.depth > 1)
    {
        depth++;
    }

    switch (depth)
    {
    case 1:
        result = VK_IMAGE_TYPE_1D;
        break;
    case 2:
        result = VK_IMAGE_TYPE_2D;
        break;
    case 3:
        result = VK_IMAGE_TYPE_3D;
        break;
    default:
        result = VK_IMAGE_TYPE_2D;
        break;
    }

    return result;
}

Image::Image(Device *device, VkImage handle, const VkExtent3D& extent, VkFormat format, VkImageUsageFlags imageUsage, VkSampleCountFlagBits sampleCount) :
    device{ device },
    handle{ handle },
    type{ ImageType(extent) },
    extent{ extent },
    format{ format },
    sampleCount{ sampleCount },
    usage{ imageUsage }
{
    subresource.mipLevel   = 1;
    subresource.arrayLayer = 1;
}

Image::Image(Device *device, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkSampleCountFlagBits sampleCount, UINT32 mipLevels, UINT32 arrayLayers, VkImageTiling tiling, VkImageCreateFlags flags, UINT32 numQueueFamilies, const UINT32* queueFamilies) :
    device{ device },
    type{ Vulkan::ImageType(extent) },
    extent{ extent },
    format{ format },
    sampleCount{ sampleCount },
    usage{ imageUsage },
    arrayLayerCount{ arrayLayers },
    tiling{ tiling }
{
    SLASSERT(mipLevels > 0 && "Image should have at least one level");
    SLASSERT(arrayLayers > 0 && "Image should have at least one level");

    subresource.mipLevel    = mipLevels;
    subresource.arrayLayer = arrayLayers;

    VkImageCreateInfo createInfo{};
    createInfo.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType   = type;
    createInfo.format      = format;
    createInfo.extent      = extent;
    createInfo.mipLevels   = mipLevels;
    createInfo.arrayLayers = arrayLayers;
    createInfo.samples     = sampleCount;
    createInfo.tiling      = tiling;
    createInfo.usage       = imageUsage;

    if (numQueueFamilies != 0)
    {
        createInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = numQueueFamilies;
        createInfo.pQueueFamilyIndices   = queueFamilies;
    }

    VmaAllocationCreateInfo memoryInfo{};
    memoryInfo.usage = memoryUsage;
    if (imageUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
    {
        memoryInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }

    Check(vmaCreateImage(device->MemoryAllocator(), &createInfo, &memoryInfo, &handle, &memory, nullptr));
}

Image::Image(Image &&other) :
    device{ other.device },
    handle{ other.handle },
    memory{ other.memory },
    type{ other.type },
    extent{ other.extent },
    format{ other.format },
    sampleCount{ other.sampleCount },
    usage{ other.usage },
    tiling{ other.tiling },
    subresource{ other.subresource },
    mappedData{ other.mappedData },
    mapped{ other.mapped }
{
    other.handle     = VK_NULL_HANDLE;
    other.memory     = VK_NULL_HANDLE;
    other.mappedData = nullptr;
    other.mapped     = false;

    for (auto& v : views)
    {
        v->Set(this);
    }
}
}
}
