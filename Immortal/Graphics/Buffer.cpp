#include "Buffer.h"

namespace Immortal
{

Buffer::Buffer() :
    _size{},
    _type{}
{

}

Buffer::Buffer(BufferType type, size_t size) :
    _size{ size },
    _type{ type }
{

}

const size_t &Buffer::GetSize() const
{
	return _size;
}

const BufferType &Buffer::GetType() const
{
	return _type;
}

void Buffer::Fill(const void *data, size_t size, uint64_t offset)
{
	void *mapped;
	Map(&mapped, size, offset);
	if (mapped)
	{
		memcpy(mapped, data, size);
		Unmap();
	}
}

void Buffer::SetDebugName(const char *name)
{
#ifdef _DEBUG
	SetName(name);
#endif
}

}
