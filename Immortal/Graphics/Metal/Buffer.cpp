#include "Buffer.h"
#include "Device.h"
#include "Foundation/NSRange.hpp"

namespace Immortal
{
namespace Metal
{

Buffer::Buffer() :
    Super{},
    Handle<MTL::Buffer>{},
    mappedSize{}
{

}

Buffer::Buffer(Device *device, Type type, size_t size, const void *data) :
    Super{ type, size },
    Handle<MTL::Buffer>{},
    mappedSize{}
{
    MTL::ResourceOptions resourceOptions = MTL::ResourceStorageModeManaged;
    handle = device->GetHandle()->newBuffer(size, resourceOptions);

    if (data)
    {
		void *mapped = nullptr;
		Map(&mapped, size, 0);
		memcpy(mapped, data, size);
		Unmap();
    }
}

Buffer::~Buffer()
{
    if (handle)
    {
        handle->release();
        handle = {};
    }
}

Anonymous Buffer::GetBackendHandle() const
{
	return (void *)handle;
}

void Buffer::Map(void **ppData, size_t size, uint64_t offset)
{
    mappedSize = size;
    uint8_t *data = (uint8_t *)handle->contents();
    *ppData = data + offset;
}

void Buffer::Unmap()
{
    handle->didModifyRange(NS::Range{ 0, mappedSize });
}

}
}
