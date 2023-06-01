#include "Buffer.h"

#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

Buffer::Buffer(uint64_t size, Type type) :
    Super{ type, size }
{
    SelectBindPoint(type);

    glCreateBuffers(1, &handle);
    glBindBuffer(bindPoint, handle);
    glBufferData(bindPoint, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(bindPoint, 0);
}

Buffer::Buffer(uint64_t size, const void *data, Type type) :
    Super{ type, size }
{
    SelectBindPoint(type);

    glCreateBuffers(1, &handle);
    glBindBuffer(bindPoint, handle);
    glBufferData(bindPoint, size, data, GL_STATIC_DRAW);
    glBindBuffer(bindPoint, 0);
}

Buffer::Buffer(uint64_t size, uint32_t binding) :
    Super{ Type::Uniform, size }
{
    SelectBindPoint(type);

    glCreateBuffers(1, &handle);
    glBindBuffer(bindPoint, handle);
    glBufferData(bindPoint, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(bindPoint, 0);
    glBindBufferRange(bindPoint, binding, handle, 0, size);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &handle);
}

void Buffer::Update(uint64_t size, const void *data, uint64_t offset)
{
    glBindBuffer(bindPoint, handle);
    glBufferSubData(bindPoint, offset, size, data);
}

UniformBuffer::UniformBuffer(uint64_t size, uint32_t binding) :
    Super{ size, binding },
    binding{ U32(binding) }
{

}

void UniformBuffer::Update(uint64_t size, const void *data, uint64_t offset)
{
    glBindBuffer(bindPoint, handle);
    glBufferSubData(bindPoint, offset, size, data);
}

}
}
