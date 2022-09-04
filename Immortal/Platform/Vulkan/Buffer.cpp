#include "Buffer.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{
#define ASSERT_ZERO_SIZE_BUFFER(size) SLASSERT(size != 0 && "Unable to Allocate a zero size buffer");

inline VkBufferUsageFlags SelectBufferUsage(Buffer::Type type)
{
    VkBufferUsageFlags flags = 0;
    if (type & Buffer::Type::Index)
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (type & Buffer::Type::Uniform)
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (type & Buffer::Type::Storage)
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (type & Buffer::Type::Vertex)
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (type & Buffer::Type::TransferSource)
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (type & Buffer::Type::TransferDestination)
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return flags;
}

Buffer::Buffer(Device *device, const size_t size, const void *data, Type type, Usage usage) :
    Super{ type, U32(size) },
    device{ device },
    persistent{ usage == Usage::Persistent }
{
    ASSERT_ZERO_SIZE_BUFFER(size);

    Create(size);
    if (data)
    {
        Update(size, data);
    }
}

Buffer::Buffer(Device *device, const size_t size, Type type, Usage usage) :
    Super{ type, U32(size) },
    device{ device },
    persistent{ usage == Usage::Persistent }
{
    ASSERT_ZERO_SIZE_BUFFER(size);

    Create(size);
}

Buffer::Buffer(Device *device, const size_t size, uint32_t binding) :
    Super{ Type::Uniform, U32(size) },
    device{ device },
    persistent{ true }
{
    ASSERT_ZERO_SIZE_BUFFER(size);
    Create(size);
}

Buffer::Buffer(const Buffer *host, const BindInfo &bindInfo) :
    Super{ bindInfo.type, bindInfo.size }
{
    THROWIF(!(bindInfo.type & host->type), "The type requested is not one of host buffer");
    descriptor.buffer = host->descriptor.buffer;
    descriptor.offset = bindInfo.offset;
    descriptor.range  = bindInfo.size;
}

Buffer::~Buffer()
{
    if (descriptor.buffer != VK_NULL_HANDLE && memory != VK_NULL_HANDLE)
    {
        struct {
            Device *device = nullptr;
            VmaAllocation memory = nullptr;
            VkBuffer buffer = VK_NULL_HANDLE;
        } dpack{
            device,
            memory,
            descriptor.buffer
        };

        device->DestroyAsync([dpack]() {
            dpack.device->Destroy(
                dpack.buffer,
                dpack.memory
                );
            });
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

    Check(device->Create(&createInfo, &allocCreateInfo, &descriptor.buffer, &memory, &allocInfo));

    if (persistent)
	{
		mappedData = static_cast<uint8_t *>(allocInfo.pMappedData);
	}
}

void Buffer::Map()
{
    if (!mappedData)
    {
        vmaMapMemory(device->MemoryAllocator(), memory, (void **)&mappedData);
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

void Buffer::Update(uint32_t size, const void *data, uint32_t offset)
{
    if (persistent)
    {
        memcpy(mappedData + offset, data, size);
        Flush();
    }
    else
    {
        Map();
        memcpy(mappedData + offset, data, size);
        Flush();
        Unmap();
    }
}

Anonymous Buffer::Descriptor() const
{
    return Anonymize(descriptor);
}

Buffer::Super *Buffer::Bind(const BindInfo &bindInfo) const
{
    return new Buffer{ this, bindInfo };
}

}
}
