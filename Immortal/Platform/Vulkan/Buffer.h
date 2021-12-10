#pragma once

#include "Common.h"
#include "Render/Buffer.h"
#include "Descriptor.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Buffer : public SuperBuffer
{
public:
    using Super  = SuperBuffer;

public:
    Buffer(Device *device, const size_t size, const void *data, Type type, Usage usage = Usage::Persistent);

    Buffer(Device *device, const size_t size, Type type, Usage usage = Usage::Persistent);

    virtual ~Buffer() override;

    virtual void Update(uint32_t size, const void *src) override;

    virtual Anonymous Descriptor() const override;

    VkDeviceSize &Offset()
    {
        return offset;
    }

    VkBuffer &Handle()
    {
        return handle;
    }

    operator VkBuffer&()
    {
        return handle;
    }

    operator VkBuffer() const
    {
        return handle;
    }

    void Create(size_t size = 0);

    void Map();

    void Unmap();

    void Flush();

    inline VkBufferUsageFlags SelectBufferUsage(Type type)
    {
        if (type == Type::Index)
        {
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (type == Type::Uniform)
        {
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

private:
    Device *device{ nullptr };

    VkBuffer handle{ VK_NULL_HANDLE };

    VmaAllocation memory{ VK_NULL_HANDLE };

    VkDeviceSize offset{ 0 };

    BufferDescriptor descriptor;

    void *mappedData{ nullptr };

    bool persistent{ false };
};

}
}
