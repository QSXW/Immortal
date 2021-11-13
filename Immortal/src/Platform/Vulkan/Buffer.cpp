#include "Buffer.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{
#define ASSERT_ZERO_SIZE_BUFFER(size) SLASSERT(size != 0 && "Unable to Allocate a zero size buffer");

Buffer::Buffer(Device *device, const size_t size, const void *data, Type type, Usage usage) :
    Super{ type, size },
    device{ device },
    persistent{ usage == Usage::Persistent }
{
    ASSERT_ZERO_SIZE_BUFFER(size);

    Create(size);
    Update(size, data);
}

Buffer::Buffer(Device *device, const size_t size, Type type, Usage usage) :
    Super{ type, size },
    device{ device },
    persistent{ usage == Usage::Persistent }
{
    ASSERT_ZERO_SIZE_BUFFER(size);

    Create(size);
}

Buffer::~Buffer()
{
    if (handle != VK_NULL_HANDLE && memory != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(device->MemoryAllocator(), handle, memory);
    }
}

void Buffer::Create(size_t size)
{
    VkBufferCreateInfo createInfo{};
    createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.usage       = SelectBufferUsage(type);
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.size        = size;

    VmaAllocationInfo allocInfo{};
    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage          = VMA_MEMORY_USAGE_GPU_TO_CPU;
    allocCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (persistent)
    {
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
    Check(vmaCreateBuffer(device->MemoryAllocator(), &createInfo, &allocCreateInfo, &handle, &memory, &allocInfo));

    offset = allocInfo.offset;

    if (persistent)
	{
		mappedData = static_cast<uint8_t *>(allocInfo.pMappedData);
	}
}

void Buffer::Map()
{
    if (!mappedData)
    {
        vmaMapMemory(device->MemoryAllocator(), memory, &mappedData);
    }
}

void Buffer::Unmap()
{
    if (mappedData)
    {
        vmaUnmapMemory(device->MemoryAllocator(), memory);
    }
    mappedData = nullptr;
}

void Buffer::Flush()
{
    vmaFlushAllocation(device->MemoryAllocator(), memory, 0, size);
}

void Buffer::Update(VkDescriptorSet descriptorSet, uint32_t biding)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = handle;
    bufferInfo.offset = offset;
    bufferInfo.range  = VK_WHOLE_SIZE;

    VkWriteDescriptorSet writeInfo{};
    writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet           = descriptorSet;
    writeInfo.dstBinding       = biding;
    writeInfo.dstArrayElement  = 0;
    writeInfo.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfo.descriptorCount  = 1;
    writeInfo.pBufferInfo      = &bufferInfo;
    writeInfo.pImageInfo       = nullptr;
    writeInfo.pTexelBufferView = nullptr;

    device->UpdateDescriptorSets(1, &writeInfo, 0, nullptr);
}

}
}
