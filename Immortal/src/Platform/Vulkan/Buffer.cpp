#include "Buffer.h"

namespace Immortal
{
namespace Vulkan
{

Buffer::Buffer(Device *device, const size_t size, const void *data, Type type, Usage usage) :
    Super{ type },
    device{ device },
    size{ size },
    persistent{ usage == Usage::Persistent }
{
    SLASSERT(size != 0 && "Unable to Allocate a zero size buffer");

    Create(size);
    Update(size, data);
}

Buffer::Buffer(Device *device, const size_t size, Type type, Usage usage) :
    Super{ type },
    device{ device },
    size{ size },
    persistent{ usage == Usage::Persistent }
{
    SLASSERT(size != 0 && "Unable to Allocate a zero size buffer");

    Create(size);
}

Buffer::~Buffer()
{
    if (handle != VK_NULL_HANDLE && memory != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(device->MemoryAllocator(), handle, memory);
    }
}

}
}
