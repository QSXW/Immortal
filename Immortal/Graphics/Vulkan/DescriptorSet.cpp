#include "DescriptorSet.h"
#include "Device.h"
#include "Buffer.h"
#include "Pipeline.h"
#include "Texture.h"
#include "Sampler.h"

namespace Immortal
{
namespace Vulkan
{

DescriptorSet::DescriptorSet(Device *device, Pipeline *pipeline) :
    device{ device },
    descriptorUpdateTemplate{ VK_NULL_HANDLE}
{
	VkDescriptorSetLayout descriptorSetLayout = pipeline->GetDescriptorSetLayout();
    Check(device->AllocateDescriptorSet(&descriptorSetLayout, &handle));

    descriptorTypes = pipeline->GetDescriptorTypes();
}

DescriptorSet::~DescriptorSet()
{

}

void DescriptorSet::Set(uint32_t slot, SuperBuffer *_buffer)
{
	Buffer *buffer = InterpretAs<Buffer>(_buffer);
    VkDescriptorBufferInfo descriptorBufferInfo = {
        .buffer = *buffer,
	    .offset = buffer->GetOffset(),
        .range  = buffer->GetSize()
    };

    VkWriteDescriptorSet writeDescriptorSet{
        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = nullptr,
        .dstSet           = handle,
        .dstBinding       = slot,
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = descriptorTypes[slot],
        .pImageInfo       = nullptr,
        .pBufferInfo      = &descriptorBufferInfo,
        .pTexelBufferView = nullptr,
    };

    device->UpdateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

void DescriptorSet::Set(uint32_t slot, SuperTexture *_texture)
{
	Texture *texture = InterpretAs<Texture>(_texture);
    VkDescriptorImageInfo descriptorImageInfo = {
        .sampler     = VK_NULL_HANDLE,
		.imageView   = texture->GetImageView(),
		.imageLayout = texture->GetLayout()
    };

    VkWriteDescriptorSet writeDescriptorSet{
        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = nullptr,
        .dstSet           = handle,
        .dstBinding       = slot,
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = descriptorTypes[slot],
        .pImageInfo       = &descriptorImageInfo,
        .pBufferInfo      = nullptr,
        .pTexelBufferView = nullptr,
    };

    device->UpdateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

void DescriptorSet::SetUavMip(uint32_t slot, SuperTexture *_texture, uint32_t mipSlice)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	if (descriptorTypes[slot] != VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	{
		return;
	}
	const uint32_t maxMip = texture->GetMipLevels() > 0u ? (uint32_t)texture->GetMipLevels() - 1u : 0u;
	const uint32_t mip = mipSlice > maxMip ? maxMip : mipSlice;
	VkDescriptorImageInfo imageInfo = texture->GetStorageDescriptorInfo(mip);

	VkWriteDescriptorSet writeDescriptorSet{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = nullptr,
		.dstSet = handle,
		.dstBinding = slot,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = descriptorTypes[slot],
		.pImageInfo = &imageInfo,
		.pBufferInfo = nullptr,
		.pTexelBufferView = nullptr,
	};

	device->UpdateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

void DescriptorSet::Set(uint32_t slot, SuperSampler *_sampler)
{
	Sampler *sampler = InterpretAs<Sampler>(_sampler);
    VkDescriptorImageInfo descriptorImageInfo = {
        .sampler     = *sampler,
		.imageView   = VK_NULL_HANDLE,
	    .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkWriteDescriptorSet writeDescriptorSet{
        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = nullptr,
        .dstSet           = handle,
        .dstBinding       = slot,
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo       = &descriptorImageInfo,
        .pBufferInfo      = nullptr,
        .pTexelBufferView = nullptr,
    };

    device->UpdateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

}
}
