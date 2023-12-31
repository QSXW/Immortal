#pragma once

#include "Common.h"
#include "Graphics/Buffer.h"
#include "Descriptor.h"
#include "Handle.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Buffer : public SuperBuffer, public Handle<VkBuffer>
{
public:
    using Super  = SuperBuffer;
	VKCPP_SWAPPABLE(Buffer)

public:
	Buffer(Device *device = nullptr);

    Buffer(Device *device, Type type, size_t size, const void *data = nullptr);

    virtual ~Buffer() override;

    Anonymous GetBackendHandle() const override;

    virtual void Map(void **ppData, size_t size, uint64_t offset) override;

    virtual void Unmap() override;

public:
    VkDeviceAddress GetDeviceAddress() const;

    void Construct();

    void Flush();

public:
    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<T, Device>())
        {
            return device;
        }
    }

    VkDeviceSize GetOffset() const
    {
        return descriptor.offset;
    }

    VkBuffer Handle() const
    {
        return descriptor.buffer;
    }

    VkBufferUsageFlags GetUsage() const
    {
		return usage;
    }

    operator VkBuffer() const
    {
        return descriptor.buffer;
    }

    void Map();

    template <class T>
    void Map(T **ppData)
    {
        Map();
        *ppData = (T *)mappedData;
    }

    VkDescriptorBufferInfo GetDescriptorInfo() const
	{
		return descriptor;
	}

    void Swap(Buffer &other)
    {
		Handle::Swap(other);
		std::swap(device,     other.device    );
		std::swap(memory,     other.memory    );
		std::swap(descriptor, other.descriptor);
		std::swap(mappedData, other.mappedData);
		std::swap(persistent, other.persistent);
    }

private:
    Device *device;

    VmaAllocation memory;

    BufferDescriptor descriptor;

    VkBufferUsageFlags usage;

    uint8_t *mappedData;

    bool persistent;
};

}
}
