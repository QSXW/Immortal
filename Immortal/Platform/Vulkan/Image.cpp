#include "impch.h"
#include "Image.h"
#include "Device.h"
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
    handle{ handle }
{
    subresource.mipLevel   = 1;
    subresource.arrayLayer = 1;
    
    info.imageType = ImageType(extent);
    info.extent    = extent;
    info.format    = format;
    info.samples   = sampleCount;
    info.usage     = imageUsage;
}

Image::Image(Device *device, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkSampleCountFlagBits sampleCount, UINT32 mipLevels, UINT32 arrayLayers, VkImageTiling tiling, VkImageCreateFlags flags, UINT32 numQueueFamilies, const UINT32* queueFamilies) :
    device{ device }
{
    SLASSERT(mipLevels > 0 && "Image should have at least one level");
    SLASSERT(arrayLayers > 0 && "Image should have at least one layer");

    subresource.mipLevel   = mipLevels;
    subresource.arrayLayer = arrayLayers;
 
    info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType     = Vulkan::ImageType(extent);
    info.format        = format;
    info.extent        = extent;
    info.mipLevels     = mipLevels;
    info.arrayLayers   = arrayLayers;
    info.samples       = sampleCount;
    info.tiling        = tiling;
    info.usage         = imageUsage;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (numQueueFamilies != 0)
    {
        info.sharingMode           = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = numQueueFamilies;
        info.pQueueFamilyIndices   = queueFamilies;
    }

    VmaAllocationCreateInfo memoryInfo{};
    memoryInfo.usage = memoryUsage;
    if (imageUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
    {
        memoryInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }

    Check(vmaCreateImage(device->MemoryAllocator(), &info, &memoryInfo, &handle, &memory, nullptr));
}

Image::Image(Device *device, VkImageCreateInfo &createInfo, VmaMemoryUsage memoryUsage)
{
    CopyProps(&this->info, &createInfo);
    VmaAllocationCreateInfo memoryInfo{};
    memoryInfo.usage = memoryUsage;

    if (info.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
    {
        memoryInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }
    Check(vmaCreateImage(device->MemoryAllocator(), &info, &memoryInfo, &handle, &memory, nullptr));
}

Image::Image(Image &&other) :
    device{ other.device },
    handle{ other.handle },
    memory{ other.memory },
    subresource{ other.subresource },
    mappedData{ other.mappedData },
    mapped{ other.mapped }
{
    CopyProps(&this->info, &other.info);

    other.handle     = VK_NULL_HANDLE;
    other.memory     = VK_NULL_HANDLE;
    other.mappedData = nullptr;
    other.mapped     = false;

    for (auto& v : views)
    {
        v->Set(this);
    }
}

Image::~Image()
{
    if (handle != VK_NULL_HANDLE && memory != VK_NULL_HANDLE)
    {
        vmaDestroyImage(device->MemoryAllocator(), handle, memory);
    }
}

Image &Image::operator=(Image &&other)
{
    SLASSERT(&other != this && "You are trying to self-assignments, which is not acceptable.");

    device          = other.device;
    handle          = other.handle;
    memory          = other.memory;
    views           = std::move(other.views);
    mappedData      = other.mappedData;
    mapped          = other.mapped;
    CopyProps(&this->info, &other.info);

    other.device = nullptr;
    other.handle = VK_NULL_HANDLE;
    other.memory = VK_NULL_HANDLE;

    return *this;
}

void Image::Map(void **dataMapped)
{
    Check(vmaMapMemory(device->MemoryAllocator(), memory, dataMapped));
}

void Image::Unmap()
{
    vmaUnmapMemory(device->MemoryAllocator(), memory);
}

}
}
