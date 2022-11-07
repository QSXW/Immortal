/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Buffer.h"

namespace Immortal
{
namespace Light
{

Buffer::Buffer(uint32_t size, Type type) :
    Super{ type, size }
{
    buffer = new uint8_t[size];
}

Buffer::Buffer(uint32_t size, const void *data, Type type) :
    Buffer(size, type)
{
    if (data)
    {
        Update(size, data, 0);
    }
}

Buffer::~Buffer()
{
    buffer.Reset();
}

void Buffer::Update(uint32_t size, const void *data, uint32_t offset)
{
    if (data)
    {
        memcpy(buffer, data, size);
    }
}

}
}
