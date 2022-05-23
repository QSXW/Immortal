/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"

namespace Immortal
{

template <class T>
class ScalarInterator
{
public:
    ScalarInterator(T *ptr) :
        ptr{ ptr }
    {}

    bool operator!=(const ScalarInterator &other) const
    {
        return ptr != other.ptr;
    }

    ScalarInterator &operator++()
    {
        ptr++;
        return *this;
    }

    T &operator*()
    {
        return *ptr;
    }

    T operator*() const
    {
        return *ptr;
    }

private:
    T *ptr;
};

template <class T>
class LightVector
{
public:
    using ValueType = T;
    using PointerType = T*;

    using Iterator = ScalarInterator<T>;

    LightVector() :
        data{},
        size{}
    {}

    LightVector(size_t size) :
        size{ size },
        data{}
    {
        AllocateMemory(size);
    }

    ~LightVector()
    {
        if (data)
        {
            delete[]data;
            data = nullptr;
        }
    }

    LightVector(LightVector &&other) :
        size{},
        data{}
    {
        other.Swap(*this);
    }

    LightVector &operator=(LightVector &&other)
    {
        LightVector{ other }.Swap(*this);
        return *this;
    }

    LightVector(const LightVector &) = delete;
    LightVector &operator=(const LightVector &) = delete;

    void Swap(LightVector &other)
    {
        std::swap(size, other.size);
        std::swap(data, other.data);
    }

    size_t Size() const
    {
        return size;
    }

    void AllocateMemory()
    {
        THROWIF(data, "The size changing is not permitted in Light Vector, try std::vector instead!");
        data = new ValueType[size];
    }

    void Resize(size_t s)
    {
        size = s;
        AllocateMemory();
    }

    T *Data() const
    {
        return data;
    }

    Iterator begin() const
    {
        return Iterator(data);
    }

    Iterator end() const
    {
        return Iterator(&data[size]);
    }

    T &operator [](size_t index)
    {
        SLASSERT(index < size);
        return data[index];
    }

    const T &operator [](size_t index) const
    {
        SLASSERT(index < size);
        return data[index];
    }

private:
    T *data;
    uint32_t size;
};

}
