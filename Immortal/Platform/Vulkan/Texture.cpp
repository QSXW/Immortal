#include "Texture.h"

#include "Render/Frame.h"
#include "GuiLayer.h"

namespace Immortal
{
namespace Vulkan
{

Texture::Texture(Device *device, const std::string &filepath, const Description &description) :
    device{ device }
{
    Frame frame{ filepath };

    width     = frame.Width();
    height    = frame.Height();
    mipLevels = CalculateMipmapLevels(width, height);

    Description desc = description;
    desc.Format = frame.Desc().Format;
    Setup(desc, frame.Size(), frame.Data());
}

Texture::Texture(Device *device, uint32_t width, uint32_t height, const void *data, const Description &description) :
    device{ device },
    Super{ width, height }
{
    if (!data)
    {
        mipLevels = 1;
    }
    Setup(description, width * height * description.FormatSize(), data);
}

Texture::~Texture()
{
    image.reset();
    view.reset();
}

void Texture::Setup(const Description &description, uint32_t size, const void *data)
{
    VkBuffer stagingBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory stagingMemory{ VK_NULL_HANDLE };

    VkBufferCreateInfo createInfo{};
    createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext       = nullptr;
    createInfo.size        = size;
    createInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    Check(device->Create(&createInfo, nullptr, &stagingBuffer));

    VkMemoryRequirements memoryRequirements{};
    device->GetRequirements(stagingBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize  = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Check(device->AllocateMemory(&allocateInfo, nullptr, &stagingMemory));
    Check(device->BindMemory(stagingBuffer, stagingMemory, 0));

    // Copy texture data into host local staging buffer
    if (data)
    {
        uint8_t *mappedData{ nullptr };
        Check(device->MapMemory(stagingMemory, 0, memoryRequirements.size, 0, rcast<void **>(&mappedData)));
        memcpy(mappedData, data, size);
        device->UnmapMemory(stagingMemory);
    }

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

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    Layout = VK_IMAGE_LAYOUT_GENERAL;

    VkImageCreateInfo imageCreateInfo{};
    ConvertType(imageCreateInfo, description);
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageCreateInfo.mipLevels     = mipLevels;
    imageCreateInfo.arrayLayers   = 1;
    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent        = { width, height, 1 };
    imageCreateInfo.usage         = usage;
    
    image.reset(new Image{
        device,
        imageCreateInfo,
        VMA_MEMORY_USAGE_GPU_ONLY
        });;

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount   = mipLevels;
    subresourceRange.layerCount   = 1;

    ImageBarrier barrier{
        *image,
        subresourceRange,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT
    }; 

    device->Transfer([&](auto copyCmd) -> void {
        copyCmd->PipelineBarrier(
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        copyCmd->CopyBufferToImage(
            stagingBuffer,
            *image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(bufferCopyRegions.size()),
            bufferCopyRegions.data()
        );

        barrier.Swap();
        barrier.To(Layout);

        copyCmd->PipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        });

    device->Destroy(stagingBuffer);
    device->FreeMemory(stagingMemory);

    SetupSampler(description);
    SetupImageView(imageCreateInfo.format);

    descriptor.Update(sampler, *view, Layout);

    descriptorSet.reset(new DescriptorSet{ device, RenderContext::DescriptorSetLayout });
    descriptorSet->Update(descriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void Texture::As(Descriptor *descriptors, size_t index)
{
    ImageDescriptor *imageDescriptors = rcast<ImageDescriptor *>(descriptors);
    imageDescriptors[index] = descriptor;
}

}
}
