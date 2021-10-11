#include "Texture.h"

#include "Render/Frame.h"

namespace Immortal
{
namespace Vulkan
{

Texture::Texture(RenderContext *context, const std::string &filepath) :
    device{ context->Get<Device*>() }
{
    Frame frame{ filepath };

    width     = frame.Width();
    height    = frame.Height();
    mipLevels = CalculateMipmapLevels(width, height);

    VkBuffer stagingBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory stagingMemory{ VK_NULL_HANDLE };

    VkBufferCreateInfo createInfo{};
    createInfo.pNext       = nullptr;
    createInfo.size        = frame.Size();
    createInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    Check(device->CreateBuffer(&createInfo, nullptr, &stagingBuffer));

    VkMemoryRequirements memoryRequirements{};
    device->GetRequirements(stagingBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.allocationSize  = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Check(device->AllocateMemory(&allocateInfo, nullptr, &stagingMemory));
    Check(device->BindBufferMemory(stagingBuffer, stagingMemory, 0));

    // Copy texture data into host local staging buffer
    uint8_t *mappedData{ nullptr };
    Check(device->MapMemory(stagingMemory, 0, memoryRequirements.size, 0, rcast<void **>(&mappedData)));
    memcpy(mappedData, frame.Data(), frame.Size());
    device->UnmapMemory(stagingMemory);

    std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

    for (int i = 0; i < mipLevels; i++)
    {
        VkBufferImageCopy bufferCopyRegion{};
        bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel       = i;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount     = 1;
        bufferCopyRegion.imageExtent.width               = width >> i;
        bufferCopyRegion.imageExtent.height              = height >> i;
        bufferCopyRegion.imageExtent.depth               = 1;
        bufferCopyRegion.imageOffset                     = VkOffset3D{ 0, 0, 0 };
        bufferCopyRegions.emplace_back(std::move(bufferCopyRegion));
    }

    VkImageCreateInfo imageCreateInfo{};
    ConvertType(imageCreateInfo, frame.Type());
    imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageCreateInfo.mipLevels     = mipLevels;
    imageCreateInfo.arrayLayers   = 1;
    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent        = { width, height, 1 };
    imageCreateInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    
    VkImage imageHandle{ VK_NULL_HANDLE };
    Check(device->CreateImage(&imageCreateInfo, nullptr, &imageHandle));
    device->GetRequirements(imageHandle, &memoryRequirements);

    allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Check(device->AllocateMemory(&allocateInfo, nullptr, &deviceMemory));
    Check(device->BindImageMemory(imageHandle, deviceMemory, 0));

    auto copyBuffer = device->Request(Level::Primary);

    device->Discard(copyBuffer);
}

Texture::~Texture()
{

}
}
}
