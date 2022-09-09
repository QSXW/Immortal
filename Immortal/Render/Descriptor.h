#pragma once

#include "Core.h"
#include "Interface/IObject.h"
#include "Algorithm/LightVector.h"

namespace Immortal
{

class DescriptorBuffer
{
public:
	DescriptorBuffer() :
	    _size{},
	    _data{}
	{

	}
	
	template <class T>
	void Request(size_t size)
	{
		_size = size;
		_data = new uint8_t[size * sizeof(T)];
		memset(_data, 0, size * sizeof(T));
	}

	template <class T>
	T *DeRef(size_t index = 0) const
	{
		THROWIF(index >= _size, SError::OutOfBound);
		return &((T *)_data.Get())[index];
	}

	size_t size() const
	{
		return _size;
	}

private:
	size_t _size;

	URef<uint8_t> _data;
};

}
