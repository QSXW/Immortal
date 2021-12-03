#include "impch.h"
#include "Buffer.h"

#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

Buffer::Buffer(size_t size, Type type) :
    Super{ type, size }
{
    SelectBindPoint(type);

    glCreateBuffers(1, &handle);
    glBindBuffer(bindPoint, handle);
    glBufferData(bindPoint, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(bindPoint, 0);
}

Buffer::Buffer(size_t size, const void *data, Type type) :
    Super{ type, size }
{
    SelectBindPoint(type);

    glCreateBuffers(1, &handle);
    glBindBuffer(bindPoint, handle);
    glBufferData(bindPoint, size, data, GL_STATIC_DRAW);
    glBindBuffer(bindPoint, 0);
}

Buffer::Buffer(size_t size, int binding) :
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

void Buffer::Update(uint32_t size, const void *data)
{
    glBindBuffer(bindPoint, handle);
    glBufferSubData(bindPoint, 0, size, data);
}

}
}
