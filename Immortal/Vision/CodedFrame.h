#pragma once

#include "Shared/IObject.h"
#include "Types.h"

#include <cstdint>
#include <vector>
#include <functional>

namespace Immortal
{
namespace Vision
{

class CodedFrame;
class IMMORTAL_API SharedCodedFrameData : public IObject
{
public:
	friend CodedFrame;

public:
	SharedCodedFrameData() :
        data{},
        size{},
	    anonymous{},
	    type{},
	    timestamp{},
        flags{},
	    release{}
    {

    }

    ~SharedCodedFrameData()
    {
        if (release)
        {
			release(InterpretAs<void>());
        }
    }

    void UpdateAnonymous()
    {
		data = anonymous.data();
		size = anonymous.size();
    }

    template <class T>
    void RefTo(const T *ptr)
    {
		anonymous.resize(sizeof(T *));
		memcpy(anonymous.data(), &ptr, sizeof(T *));
    }

    void Assign(std::vector<uint8_t> &&other)
	{
		anonymous = std::move(other);
		UpdateAnonymous();
	}

    template <class T>
	void Assign(const T *ptr, size_t size)
    {
		anonymous.resize(size);
		memcpy(anonymous.data(), ptr, size);
		UpdateAnonymous();
    }

    template <class T>
	T *InterpretAs() const
	{
		return *(T **)anonymous.data();
	}

    operator bool() const
    {
        return !!data;
    }

    void SetRelease(std::function<void(void *)> &&func)
    {
		release = func;
    }

    const int64_t &GetTimestamp() const
    {
		return timestamp;
    }

    void SetTimestamp(int64_t value)
    {
		timestamp = value;
    }

protected:
	uint8_t *data;

    size_t size;

	std::vector<uint8_t> anonymous;

	MediaType type;

    int64_t timestamp;

    PictureFlags flags;

	std::function<void(void *)> release;
};

class IMMORTAL_API CodedFrame
{
public:
	CodedFrame() :
	    _shared{}
    {

    }

    template <class T>
    CodedFrame(const T *data) :
	    _shared{ new SharedCodedFrameData }
    {
		_shared->RefTo(data);
    }

    template <class T>
	CodedFrame(const T *data, size_t size) :
	    _shared{new SharedCodedFrameData}
	{
		_shared->data = (uint8_t *)data;
		_shared->size = size;
	}

	CodedFrame(std::vector<uint8_t> &&data) :
	    _shared{new SharedCodedFrameData}
	{
		_shared->Assign(std::move(data));
	}

    ~CodedFrame()
    {

    }

    template <class T>
    void SetAnonymous(T *obj)
    {
		_shared->RefTo(obj);
    }

    void SetType(MediaType value)
	{
		_shared->type = value;
	}

	const MediaType &GetType() const
	{
		return _shared->type;
	}

    void Assign(std::vector<uint8_t> &&other)
    {
		_shared->Assign(std::move(other));
    }

    template <class T>
    void RefTo(const T *ptr)
    {
		_shared->RefTo(ptr);
    }

    template <class T>
	T *InterpretAs() const
	{
		return _shared->InterpretAs<T>();
	}

    operator bool() const
    {
		return _shared && *_shared;
    }

    void SetRelease(std::function<void(void *)> &&func)
    {
		_shared->SetRelease(std::move(func));
    }

    size_t GetSize() const
    {
        if (!_shared)
        {
			return 0;
        }

        return _shared->size;
    }

    const uint8_t *GetData() const
	{
		if (!_shared)
		{
			return nullptr;
		}

		return _shared->data;
	}

    const int64_t &GetTimestamp() const
	{
		return _shared->GetTimestamp();
	}

	void SetTimestamp(int64_t value)
	{
		_shared->SetTimestamp(value);
	}

    void SetFlags(PictureFlags flags)
	{
		_shared->flags |= flags;
	}

	PictureFlags GetFlags() const
	{
		return _shared->flags;
	}

public:
	Ref<SharedCodedFrameData> _shared;
};

}

using CodedFrame = Vision::CodedFrame;

}
