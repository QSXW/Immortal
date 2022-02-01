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

    Buffer(Device *device, const size_t size, uint32_t binding);

    Buffer(const Buffer *host, const BindInfo &bindInfo);

    virtual ~Buffer() override;

    virtual void Update(uint32_t size, const void *src, uint32_t offset = 0) override;

    virtual Anonymous Descriptor() const override;

    virtual Buffer::Super *Bind(const BindInfo &bindInfo) const override;

    VkDeviceSize &Offset()
    {
        return descriptor.offset;
    }

    VkBuffer &Handle()
    {
        return descriptor.buffer;
    }

    operator VkBuffer() const
    {
        return descriptor.buffer;
    }

    void Create(size_t size = 0);

    void Map();

    void Unmap();

    void Flush();

private:
    Device *device{ nullptr };

    VmaAllocation memory{ VK_NULL_HANDLE };

    BufferDescriptor descriptor;

    void *mappedData{ nullptr };

    bool persistent{ false };
};

}
}
