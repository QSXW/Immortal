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

Image::Image() :
    handle{},
    device{}
{

}

Image::Image(Device *device, VkImage handle, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags imageUsage, VkSampleCountFlagBits sampleCount) :
    device{ device },
    handle{ handle }
{
    properties.imageType   = ImageType(extent);
    properties.extent      = extent;
    properties.format      = format;
    properties.samples     = sampleCount;
    properties.usage       = imageUsage;
    properties.arrayLayers = 1;
    properties.mipLevels   = 1;
}

Image::Image(Device *device, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkSampleCountFlagBits sampleCount, uint32_t mipLevels, uint32_t arrayLayers, VkImageTiling tiling, VkImageCreateFlags flags, uint32_t numQueueFamilies, const uint32_t* queueFamilies) :
    device{ device }
{
    SLASSERT(mipLevels > 0 && "Image should have at least one level");
    SLASSERT(arrayLayers > 0 && "Image should have at least one layer");
 
    properties.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    properties.imageType     = ImageType(extent);
    properties.format        = format;
    properties.extent        = extent;
    properties.mipLevels     = mipLevels;
    properties.arrayLayers   = arrayLayers;
    properties.samples       = sampleCount;
    properties.tiling        = tiling;
    properties.usage         = imageUsage;
    properties.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (numQueueFamilies != 0)
    {
        properties.sharingMode           = VK_SHARING_MODE_CONCURRENT;
        properties.queueFamilyIndexCount = numQueueFamilies;
        properties.pQueueFamilyIndices   = queueFamilies;
    }

    Instantiate(memoryUsage);
}

Image::Image(Device *device, const VkImageCreateInfo &createInfo, VmaMemoryUsage memoryUsage) :
    device{ device }
{
    Invalidate(createInfo, memoryUsage);
}

Image::Image(Image &&other) :
    device{},
    handle{},
    memory{}
{
    other.Swap(*this);
    CopyProps(&properties, &other.properties);
}

Image::~Image()
{
    Destroy();
}

void Image::Invalidate(const VkImageCreateInfo &createInfo, VmaMemoryUsage memoryUsage)
{
    properties = createInfo;
    Instantiate(memoryUsage);
}

void Image::Instantiate(VmaMemoryUsage memoryUsage)
{
    Destroy();

    VmaAllocationCreateInfo memoryInfo{};
    memoryInfo.usage = memoryUsage;

    if (properties.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
    {
        memoryInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }

    Check(device->Create(&properties, &memoryInfo, &handle, &memory));
}

void Image::Destroy()
{
    if (handle != VK_NULL_HANDLE && memory != VK_NULL_HANDLE && device != nullptr)
    {
        struct {
            Device *device = nullptr;
            VmaAllocation memory = nullptr;
            VkImage image = VK_NULL_HANDLE;
        } dpack{
            device,
            memory,
            handle
        };

        device->DestroyAsync([dpack]() {
            dpack.device->Destroy(
                dpack.image,
                dpack.memory
            );
            });
    }
}

Image &Image::operator=(Image &&other)
{
    Image(std::move(other)).Swap(*this);
    CopyProps(&properties, &other.properties);
    return *this;
}

void Image::Swap(Image &other)
{
    std::swap(device, other.device);
    std::swap(handle, other.handle);
    std::swap(memory, other.memory);
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
