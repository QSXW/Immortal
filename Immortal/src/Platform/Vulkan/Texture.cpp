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
    createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext       = nullptr;
    createInfo.size        = frame.Size();
    createInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    Check(device->Create(&createInfo, nullptr, &stagingBuffer));

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
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
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

    image = std::make_unique<Image>(device, imageHandle, imageCreateInfo.extent, imageCreateInfo.format, imageCreateInfo.usage);

    auto copyCmd = device->BeginUpload();

    VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount   = mipLevels;
	subresourceRange.layerCount   = 1;

	// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
    VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.image            = image->Handle();
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.srcAccessMask    = 0;
	imageMemoryBarrier.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    copyCmd->PipelineBarrier(
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier
        );

    copyCmd->CopyBufferToImage(
		stagingBuffer,
		image->Handle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data()
        );

    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    copyCmd->PipelineBarrier(
	    VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	    0,
	    0, nullptr,
	    0, nullptr,
	    1, &imageMemoryBarrier
        );
    layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    device->EndUpload(copyCmd);

    device->Destory(stagingBuffer);
    device->FreeMemory(stagingMemory);

    INITSampler(frame.Type());
    INITImageView();
    INITDescriptor();
}

Texture::~Texture()
{
    device->FreeMemory(deviceMemory);
}

void Texture::INITDescriptor()
{
    std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings = {
		// Binding 0 : Vertex shader uniform buffer
		VkDescriptorSetLayoutBinding{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT,
			1,
			nullptr
		},
		// Binding 1 : Fragment shader image sampler
		VkDescriptorSetLayoutBinding{
			1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT,
			1,
			nullptr
		}
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo{};
    descriptorLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCreateInfo.bindingCount = setLayoutBindings.size();
	descriptorLayoutCreateInfo.pBindings    = setLayoutBindings.data();
	Check(device->CreateDescriptorSetLayout(&descriptorLayoutCreateInfo, nullptr, &descriptorSetLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pSetLayouts    = &descriptorSetLayout;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	Check(device->CreatePipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
    Check(device->AllocateDescriptorSet(&descriptorSetLayout, &descriptorSet));

	VkDescriptorImageInfo textureDescriptor{};
	textureDescriptor.imageView   = view->Handle();
	textureDescriptor.sampler     = sampler.Handle();
	textureDescriptor.imageLayout = layout;

    VkWriteDescriptorSet writeDesc{};
    writeDesc.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDesc.dstSet          = descriptorSet;
    writeDesc.descriptorCount = 1;
    writeDesc.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDesc.pImageInfo      = &textureDescriptor;

    device->UpdateDescriptorSets(1, &writeDesc, 0, nullptr);
}
}
}
