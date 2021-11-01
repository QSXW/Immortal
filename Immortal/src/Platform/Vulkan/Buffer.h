#pragma once

#include "Render/Buffer.h"
#include "Common.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

class Buffer : public SuperBuffer
{
public:
    using Super = SuperBuffer;

public:
    Buffer(Device *device, const size_t size, const void *data, Type type, Usage usage = Usage::Persistent);

    Buffer(Device *device, const size_t size, Type type, Usage usage = Usage::Persistent);

    virtual ~Buffer() override;

    void Create(size_t size = 0)
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

        if (persistent)
	    {
		    mappedData = static_cast<uint8_t *>(allocInfo.pMappedData);
	    }
    }

    void Map()
    {
        if (!mappedData)
        {
            vmaMapMemory(device->MemoryAllocator(), memory, &mappedData);
        }
    }

    void Unmap()
    {
        if (mappedData)
        {
           vmaUnmapMemory(device->MemoryAllocator(), memory);
        }
        mappedData = nullptr;
    }

    void Flush()
    {
        vmaFlushAllocation(device->MemoryAllocator(), memory, 0, size);
    }

    void Update(size_t size, const void *src)
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

    static inline VkBufferUsageFlags SelectBufferUsage(Type type)
    {
        if (type == Type::Index)
        {
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

private:
    Device *device{ nullptr };

    VkBuffer handle{ VK_NULL_HANDLE };

    VmaAllocation memory{ VK_NULL_HANDLE };

    size_t size;

    void *mappedData{ nullptr };

    bool persistent{ false };
};

}
}
