/**
 * Copyright (C) 2023, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include <memory_resource>
#include <list>
#include <immintrin.h>

#include "Allocator.h"
#include "Interface/IObject.h"

namespace Immortal
{
   
class MarkBuffer
{
public:
    MarkBuffer(uint8_t *data, size_t count) :
        data{ data },
        mark{((uint64_t)2 << (count - 1)) - 1}
    {

    }

    void Unmark(size_t index)
    {
        if (index == 0)
        {
            mark &= (uint64_t)~1;
        }
        else
        {
            mark ^= ((uint64_t) 2 << (index - 1));
        }
    }

    void Mark(size_t index)
    {
        if (index == 0)
        {
            mark |= 1;
        }
        else
        {
            mark |= ((uint64_t) 2 << (index - 1));
        }
    }

    uint8_t *Allocate(size_t size)
    {
        if (!mark)
        {
            return nullptr;
        }

        auto index = _tzcnt_u64(mark);
        Unmark(index);
        return &data[index * size];
    }

    bool Release(uint8_t *ptr,size_t size, size_t count)
    {
        if (ptr >= data && ptr < data + size * count)
        {
            Mark((ptr - data) / size); 
            return true;
        }

        return false;
    }

public:
    uint8_t *data;

    uint64_t mark;
};

class MemoryResource
{
public:
    MemoryResource(size_t size, size_t count = 64) :
        size{size},
        count{count}
    {

    }

    ~MemoryResource()
    {
		for (auto &buffer : buffers)
        {
            allocator.deallocate(buffer.data);
			buffer.data = nullptr;
        }
    }

    void *Allocate()
    {
        for (auto &buffer : buffers)
        {
            uint8_t *ptr = buffer.Allocate(size);
            if (ptr)
            {
                return ptr;
            }
        }
        
        uint8_t *data = allocator.allocate(size * count);
        auto &buffer = buffers.emplace_back(MarkBuffer{ data, count});
        return buffer.Allocate(size);
    }

    void Release(void *ptr)
    {
        for (auto &buffer : buffers)
        {
            if (buffer.Release((uint8_t *)ptr, size, count))
            {
                return;
            }
        }
    }

protected:
    AAllocator<uint8_t> allocator;

    std::list<MarkBuffer> buffers;

    size_t size;

    size_t count;
};

}
