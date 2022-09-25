/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"

namespace Immortal
{

template <class T, size_t Capacity>
class Rotate
{
public:
    using Iterator = ScalarInterator<T>;

    using ConstantInterator = ConstantScalarInterator<T>;

public:
	Rotate() :
		_data{},
	    _sync{}
	{

	}

	operator T&()
	{
		T &ret = _data[_sync];
		SLROTATE(_sync, Capacity);
		return ret;
	}

	const size_t size() const
	{
		return Capacity;
	}

	T *data() const
	{
		return _data;
	}

	T &operator[](size_t index)
	{
		return _data[index];
	}

	Iterator begin()
	{
		return Iterator(_data);
	}

	Iterator end()
	{
		return Iterator(&_data[Capacity]);
	}

	ConstantInterator begin() const
	{
		return ConstantInterator(&_data[0]);
	}

	ConstantInterator end() const
	{
		return ConstantInterator(&_data[Capacity]);
	}

protected:
	T _data[Capacity];

	uint32_t _sync;
};

}
