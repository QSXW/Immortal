/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"
#include "LightVector.h"

namespace Immortal
{

template <class T, size_t Reserved = 16>
class LightArray
{
public:
    static constexpr size_t Capacity = Reserved;

    using value_type = T;

    using Iterator = ScalarInterator<T>;

    using ConstantInterator = ConstantScalarInterator<T>;

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

    void resize(size_t size) const
    {
		_size = size;
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

    void emplace_back(T &&object)
	{
		SLASSERT(_size < Capacity && "Too big size for current LightArray."
		                             "Please specify a bigger capacity for LightArray or use LightVector instead.");
		_data[_size++] = std::move(object);
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

    bool empty() const
    {
        return _size == 0;
    }

    Iterator begin()
    {
        return Iterator(_data);
    }

    Iterator end()
    {
        return Iterator(&_data[_size]);
    }

    ConstantInterator begin() const
    {
        return ConstantInterator(&_data[0]);
    }

    ConstantInterator end() const
    {
        return ConstantInterator(&_data[_size]);
    }

private:
    size_t _size;

    T _data[Capacity];
};

}
