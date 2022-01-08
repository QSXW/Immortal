#include "Buffer.h"
#include "Device.h"
#include "..\OpenGL\Buffer.h"

namespace Immortal
{
namespace Vulkan
{
#define ASSERT_ZERO_SIZE_BUFFER(size) SLASSERT(size != 0 && "Unable to Allocate a zero size buffer");

inline VkBufferUsageFlags SelectBufferUsage(Buffer::Type type)
{
    if (type == Buffer::Type::Index)
    {
        return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (type == Buffer::Type::Uniform)
    {
        return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }
    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
}

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

Buffer::Buffer(Device *device, const size_t size, uint32_t binding) :
    Super{ Type::Uniform, size },
    device{ device },
    persistent{ true }
{
    ASSERT_ZERO_SIZE_BUFFER(size);
    Create(size);
}

Buffer::~Buffer()
{
    device->Wait();
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
    allocCreateInfo.usage          = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (persistent)
    {
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
    Check(vmaCreateBuffer(device->MemoryAllocator(), &createInfo, &allocCreateInfo, &handle, &memory, &allocInfo));

    // offset = allocInfo.offset;

    if (persistent)
	{
		mappedData = static_cast<uint8_t *>(allocInfo.pMappedData);
	}
    descriptor.Update(handle, offset, VK_WHOLE_SIZE);
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

void Buffer::Update(uint32_t size, const void *src)
{
    if (persistent)
    {
        memcpy(mappedData, src, size);
        Flush();
    }
    else
    {
        Map();
        memcpy(mappedData, src, size);
        Flush();
        Unmap();
    }
}

Anonymous Buffer::Descriptor() const
{
    return Anonymize(descriptor);
}

}
}
