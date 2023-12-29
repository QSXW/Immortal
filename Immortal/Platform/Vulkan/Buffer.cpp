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
    if (type & Buffer::Type::ConstantBuffer)
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (type & (Buffer::Type::Storage | Buffer::Type::Scratch))
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
    if (type & Buffer::Type::AccelerationStructureSource)
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    if (type & Buffer::Type::AccelerationStructure)
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    }

    return flags;
}

Buffer::Buffer(Device *device) :
    Super{},
    device{ device },
    memory{ VK_NULL_HANDLE },
    descriptor{},
    usage{},
    mappedData{},
    persistent{}
{

}

Buffer::Buffer(Device *device, Type type, size_t size, const void *data) :
    Super{ type, size },
    device{ device },
    memory{VK_NULL_HANDLE},
    descriptor{},
    usage{},
    mappedData{},
    persistent{ false }
{
    ASSERT_ZERO_SIZE_BUFFER(size);

    Construct();
    if (data)
    {
		void *mapped = nullptr;
		Map(&mapped);
		memcpy(mapped, data, size);
		Unmap();
    }
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
            handle
        };

        device->DestroyAsync([dpack]() {
            dpack.device->Destroy(
                dpack.buffer,
                dpack.memory
                );
            });

        handle = VK_NULL_HANDLE;
		memory = VK_NULL_HANDLE;
		device = nullptr;
    }
}

Anonymous Buffer::GetBackendHandle() const
{
	return (void *)handle;
}

void Buffer::Construct()
{
    VkBufferCreateInfo createInfo{};
    createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.usage       = usage = SelectBufferUsage(GetType());
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.size        = GetSize();

    if (device->IsEnabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
    {
        createInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    VmaAllocationInfo allocInfo{};
    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage          = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (persistent)
    {
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    Check(device->Create(&createInfo, &allocCreateInfo, &handle, &memory, &allocInfo));
	descriptor.buffer = handle;

    if (persistent)
	{
		mappedData = static_cast<uint8_t *>(allocInfo.pMappedData);
	}
}

void Buffer::Map(void **ppData, size_t size, uint64_t offset)
{
	if (!mappedData && !persistent)
	{
		vmaMapMemory(device->MemoryAllocator(), memory, (void **) &mappedData);
	}
	*ppData = (uint8_t *)mappedData + offset;
}

void Buffer::Map()
{
    if (!mappedData && !persistent)
    {
        device->MapMemory(memory, (void **)&mappedData);
    }
}

void Buffer::Unmap()
{
    if (mappedData && !persistent)
    {
		device->UnmapMemory(memory);
        mappedData = nullptr;
    }
}

void Buffer::Flush()
{
    vmaFlushAllocation(device->MemoryAllocator(), memory, 0, GetSize());
}

VkDeviceAddress Buffer::GetDeviceAddress() const
{
    return device->GetBufferAddress(descriptor.buffer);
}

}
}
