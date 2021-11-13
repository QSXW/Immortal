#pragma once

#include "Common.h"
#include "Render/Buffer.h"

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

    void Update(VkDescriptorSet descriptorSet, uint32_t biding);

    virtual void Update(uint32_t size, const void *src) override
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

    void *mappedData{ nullptr };

    bool persistent{ false };
};

}
}
