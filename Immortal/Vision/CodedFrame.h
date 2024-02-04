#pragma once

#include "Shared/IObject.h"

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
	    buffer{},
	    type{},
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

    void Assign(std::vector<uint8_t> &&other)
    {
        buffer = std::move(other);
    }

    template <class T>
    void RefTo(const T *ptr)
    {
        buffer.resize(sizeof(T *));
        memcpy(buffer.data(), &ptr, sizeof(T *));
    }

    template <class T>
	T *InterpretAs() const
	{
		return *(T **)buffer.data();
	}

    operator bool() const
    {
        return !buffer.empty();
    }

    const std::vector<uint8_t> &GetBuffer() const
    {
		return buffer;
    }

    void SetRelease(std::function<void(void *)> &&func)
    {
		release = func;
    }

protected:
	std::vector<uint8_t> buffer;

	MediaType type;

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

	CodedFrame(std::vector<uint8_t> &&data) :
	    _shared{new SharedCodedFrameData}
	{
		_shared->Assign(std::move(data));
	}

    ~CodedFrame()
    {

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
		return !_shared->GetBuffer().empty();
    }

    const std::vector<uint8_t> &GetBuffer() const
    {
		return _shared->GetBuffer();
    }

    void SetRelease(std::function<void(void *)> &&func)
    {
		_shared->SetRelease(std::move(func));
    }

public:
	Ref<SharedCodedFrameData> _shared;
};

}
}
