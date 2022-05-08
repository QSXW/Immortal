#include "Texture.h"

#include "Render/Frame.h"
#include "GuiLayer.h"
#include "Device.h"
#include "RenderContext.h"

namespace Immortal
{
namespace Vulkan
{

Texture::Texture(Device *device, const std::string &filepath, const Description &description) :
    device{ device },
    Super{ filepath }
{
    desc = description;

    Frame frame{ filepath };
    if (!frame.Available())
    {
        return;
    }

    Vision::Picture picture = frame.GetPicture();

    width     = picture.desc.width;
    height    = picture.desc.height;
    mipLevels = desc.mipLevels ? CalculateMipmapLevels(width, height) : 1;

    desc.format = picture.desc.format;
    Setup(desc, picture.desc.Size(), picture.Data());
}

Texture::Texture(Device *device, uint32_t width, uint32_t height, const void *data, const Description &description, uint32_t layer) :
    Super{ width, height },
    device{ device },
    desc{ description }
{
    if (width == 0 || height == 0)
    {
        return;
    }

    Super::width  = width;
    Super::height = height;
    mipLevels = desc.mipLevels ? CalculateMipmapLevels(width, height) : 1;

    Setup(description, width * height * description.format.Size(), data, layer);
}

Texture::~Texture()
{
    image.Release();
    view.Release();
}

void Texture::Setup(const Description &description, uint32_t size, const void *data, uint32_t layers)
{
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    Layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.flags         = layers == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageCreateInfo.mipLevels     = mipLevels;
    imageCreateInfo.arrayLayers   = layers;
    imageCreateInfo.format        = description.format;
    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent        = { width, height, 1 };
    imageCreateInfo.usage         = usage;

    image = new Image{
        device,
        imageCreateInfo,
        VMA_MEMORY_USAGE_GPU_ONLY
        };

    if (data)
    {
        InternalUpdate((void*)data);
    }
    Synchronize();

    sampler = new Sampler{ device, desc, mipLevels };

    view = new ImageView{
         image,
         layers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_CUBE,
         imageCreateInfo.format,
         0, 0,
         mipLevels,
         layers
        };

    descriptor.Update(*sampler, *view, Layout);

    descriptorSet = new DescriptorSet{ device, RenderContext::DescriptorSetLayout };
    descriptorSet->Update(descriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void Texture::Update(void *data)
{
    InternalUpdate(data);
    Synchronize();
}

void Texture::InternalUpdate(void *data)
{
    size_t size = width * height * desc.format.Size();
    Buffer stagingBuffer{ device, size, data, Buffer::Type::TransferSource };

    std::vector<VkBufferImageCopy> bufferCopyRegions;

    uint32_t layers = LayerCount();
    VkBufferImageCopy bufferCopyRegion{};
    bufferCopyRegion.bufferOffset                    = 0;
    bufferCopyRegion.bufferRowLength                 = 0;
    bufferCopyRegion.bufferImageHeight               = 0;
    bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel       = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount     = 1;
    bufferCopyRegion.imageExtent.width               = width;
    bufferCopyRegion.imageExtent.height              = height;
    bufferCopyRegion.imageExtent.depth               = 1;
    bufferCopyRegion.imageOffset                     = VkOffset3D{ 0, 0, 0 };
    bufferCopyRegions.emplace_back(std::move(bufferCopyRegion));

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel   = 0;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.levelCount     = 1;
    subresourceRange.layerCount     = 1;

    ImageBarrier barrier{
        *image,
        subresourceRange,
        Layout,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT
    };

    device->TransferAsync([&](CommandBuffer *copyCmd) -> void {
        copyCmd->PipelineImageBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            &barrier
         );

        copyCmd->CopyBufferToImage(
            stagingBuffer,
            *image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(bufferCopyRegions.size()),
            bufferCopyRegions.data()
            );

        Layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.Swap();
        barrier.To(Layout);

        copyCmd->PipelineImageBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            &barrier
            );
        });

    GenerateMipMaps();
}

void Texture::GenerateMipMaps()
{
    uint32_t layers = image->Info().arrayLayers;
    device->TransferAsync([&](CommandBuffer *cmdbuf) {
        for (uint32_t i = 1; i < mipLevels; i++)
        {
            VkImageBlit imageBlit{};

            uint32_t srcMipLevel = i - 1;
            imageBlit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.baseArrayLayer = 0;
            imageBlit.srcSubresource.layerCount     = layers;
            imageBlit.srcSubresource.mipLevel       = srcMipLevel;
            imageBlit.srcOffsets[1].x               = width >> srcMipLevel;
            imageBlit.srcOffsets[1].y               = height >> srcMipLevel;
            imageBlit.srcOffsets[1].z               = 1;

            imageBlit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.baseArrayLayer = 0;
            imageBlit.dstSubresource.layerCount     = layers;
            imageBlit.dstSubresource.mipLevel       = i;
            imageBlit.dstOffsets[1].x               = width >> i;
            imageBlit.dstOffsets[1].y               = height >> i;
            imageBlit.dstOffsets[1].z               = 1;

            VkImageSubresourceRange mipSubRange = {};
            mipSubRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
            mipSubRange.baseMipLevel = i;
            mipSubRange.levelCount   = 1;
            mipSubRange.layerCount   = layers;

            ImageBarrier barrier{
                *image,
                mipSubRange,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT
            };

            cmdbuf->PipelineImageBarrier(
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                &barrier
            );

            cmdbuf->BlitImage(
                *image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                *image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageBlit,
                VK_FILTER_LINEAR
            );

            // Prepare current mip level as image blit source for next level
            Layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.Swap();
            barrier.To(Layout);
            barrier.To(VK_ACCESS_TRANSFER_READ_BIT);

            cmdbuf->PipelineImageBarrier(
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                &barrier
            );
        }
    });
}

void Texture::Synchronize()
{
    const auto &info = image->Info();
    device->TransferAsync([&](CommandBuffer *cmdbuf) {
        VkImageLayout newLayout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        if (Layout == VK_IMAGE_LAYOUT_UNDEFINED /* Which means the image is not created with data updated */)
        {
            newLayout = VK_IMAGE_LAYOUT_GENERAL;
            VkImageSubresourceRange subresourceRange{};
            subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel   = 0;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.levelCount     = 1;
            subresourceRange.layerCount     = info.arrayLayers;

            ImageBarrier barrier{
                *image,
                subresourceRange,
                Layout,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT
            };

            cmdbuf->PipelineImageBarrier(
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                &barrier
            );
        }

        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.levelCount = mipLevels;
        subresourceRange.layerCount = info.arrayLayers;
    
        ImageBarrier barrier = {
            *image,
            subresourceRange,
            Layout,
            newLayout,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            0
        };
     
        Layout = newLayout;

        cmdbuf->PipelineImageBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            &barrier
        );
    });
}

void Texture::As(Descriptor *descriptors, size_t index)
{
    ImageDescriptor *imageDescriptors = rcast<ImageDescriptor *>(descriptors);
    imageDescriptors[index] = descriptor;
}

Texture::operator uint64_t() const
{
    return *descriptorSet;
}

bool Texture::operator==(const Texture::Super &super) const
{
    const Texture &other = dynamic_cast<const Texture &>(super);
    return image == other.image;
}

TextureCube::TextureCube(Device * device, uint32_t width, uint32_t height, const Description &desc) :
    Vulkan::Texture{ device, width, height, nullptr, desc, 6 }
{

}

}
}
