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

    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    if (buffer->GetUsage() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    {
		descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    VkWriteDescriptorSet writeDescriptorSet{
        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = nullptr,
        .dstSet           = handle,
        .dstBinding       = slot,
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = descriptorType,
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

    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    if (texture->GetUsage() & VK_IMAGE_USAGE_STORAGE_BIT)
    {
		descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    VkWriteDescriptorSet writeDescriptorSet{
        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = nullptr,
        .dstSet           = handle,
        .dstBinding       = slot,
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo       = &descriptorImageInfo,
        .pBufferInfo      = nullptr,
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
