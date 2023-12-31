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

}
