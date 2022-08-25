/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"

namespace Immortal
{

template <class T, size_t Reserved = 16>
class LightArray
{
public:
    static constexpr size_t Capacity = Reserved;

    using value_type = typename T;

public:
    LightArray() :
        _size{},
        _data{}
    {
    
    }

    ~LightArray()
    {
    
    }

    /**
     * @brief Align to stl 
     */
    size_t size() const
    {
        return _size;
    }

    T *data()
    {
        return _data;
    }

    size_t capacity() const
    {
        return Capacity;
    }

    void emplace_back(const T &object)
    {
        SLASSERT(_size < Capacity && "Too big size for current LightArray."
                                     "Please specify a bigger capacity for LightArray or use LightVector instead.");
        _data[_size++] = object;
    }

    T &operator[](size_t index)
    {
        SLASSERT(index < _size);
        return _data[index];
    }

    void clear()
    {
        _size = 0;
    }

private:
    size_t _size;

    T _data[Capacity];
};

}
