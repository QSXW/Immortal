#include "Buffer.h"

#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

static Buffer::BindPointType SelectBindPoint(BufferType type)
{
	switch (type)
	{
		case BufferType::Index:
			return GL_ELEMENT_ARRAY_BUFFER;
			break;

		case BufferType::Vertex:
			return GL_ARRAY_BUFFER;

        case BufferType::TransferSource:
			return GL_COPY_READ_BUFFER;

        case BufferType::TransferDestination:
			return GL_COPY_WRITE_BUFFER;

        case BufferType::ConstantBuffer:
		default:
			return GL_UNIFORM_BUFFER;
	}
}

Buffer::Buffer(uint64_t size, Type type) :
    Super{ type, size },
    bindPoint{ SelectBindPoint(type) }
{
    if (type == BufferType::TransferSource || type == BufferType::TransferDestination)
    {
		memory.resize(size);
		return;
    }

    glCreateBuffers(1, &handle);
    glBindBuffer(bindPoint, handle);
    glBufferData(bindPoint, size, nullptr, GL_STREAM_DRAW);
    glBindBuffer(bindPoint, 0);
}

Buffer::Buffer(uint64_t size, const void *data, Type type) :
    Super{ type, size },
    Handle{}
{
    SelectBindPoint(type);

    glCreateBuffers(1, &handle);
    glBindBuffer(bindPoint, handle);
    glBufferData(bindPoint, size, data, GL_STATIC_DRAW);
    glBindBuffer(bindPoint, 0);
}

Buffer::~Buffer()
{
	if (handle)
    {
		glDeleteBuffers(1, &handle);
    }
}

Anonymous Buffer::GetBackendHandle() const
{
	return (void *)(uint64_t)handle;
}

void Buffer::Map(void **ppData, size_t size, uint64_t offset)
{
    if (handle)
    {
		glBindBuffer(bindPoint, handle);
		*ppData = glMapBuffer(bindPoint, GL_READ_WRITE);
		return;
    }

    if (memory.empty())
    {
		memory.resize(size);
    }

    *ppData = &memory[offset];
}

void Buffer::Unmap()
{
    if (handle)
	{
		glUnmapBuffer(bindPoint);
		glBindBuffer(bindPoint, Handle::Invalid);
    }
}

void Buffer::Update(uint64_t size, const void *data, uint64_t offset)
{
	glBindBuffer(bindPoint, handle);
	glBufferSubData(bindPoint, offset, size, data);
	glBindBuffer(bindPoint, Handle::Invalid);
}

}
}
