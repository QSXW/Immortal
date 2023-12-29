#pragma once

#include "Format.h"
#include "Interface/IObject.h"
#include "Vision/Types.h"
#include "Vision/Common/Error.h"
#include "Vision/Common/Animator.h"
#include "Framework/Async.h"
#include "Memory/MemoryResource.h"

namespace Immortal
{
namespace Vision
{

struct CodedFrame
{
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
    T *DeRef() const
    {
        return *(T **)buffer.data();
    }

    operator bool() const
    {
        return !buffer.empty();
    }

public:
    std::vector<uint8_t> buffer;

    MediaType Type;
};

using TimeStamp = uint64_t;

enum class PictureType
{
    System,
    Device,
};

/** Shared Picture Data
 *
 *  This struct only can used as a shared pointer
 *  Construct once and Deconstruct once
 */
struct SharedPictureData : IObject
{
	SharedPictureData(MemoryResource *memoryResource = nullptr) :
	    memoryResource{memoryResource},
        data{},
        linesize{},
	    type{},
	    colorSpace{},
        destory{}
    {

    }

    SharedPictureData(int width, int height, Format format, MemoryResource *memoryResource = nullptr) :
	    SharedPictureData{ memoryResource }
    {
		auto ptr = new uint8_t[width * height * format.Size()];
        destory = [=] {
			delete []ptr;
        };
		data[0] = ptr;
    }

    ~SharedPictureData()
    {
		if (destory)
		{
			destory();
		}
    }

    MemoryResource *memoryResource;

    uint8_t *data[8];

    int linesize[8];

	PictureType type;

    ColorSpace colorSpace;

    std::function<void()> destory;
};

struct Picture
{
    Picture() :
        desc{},
        shared{},
        pts{}
    {

    }

    ~Picture()
    {
		ReleaseSharedData();
    }

	Picture(int width, int height, Format format, MemoryResource *memoryResource = nullptr, bool allocated = false) :
	    desc{ width, height, format },
	    shared{},
	    pts{}
    {
		if (memoryResource)
		{
			void *object = memoryResource->Allocate();
			if (allocated)
			{
				shared = new (object) SharedPictureData{width, height, format, memoryResource};
			}
			else
			{
				shared = new (object) SharedPictureData{memoryResource};
			}
		}
		else
		{
			if (allocated)
			{
				shared = new SharedPictureData{width, height, format};
			}
			else
			{
				shared = new SharedPictureData{};
			}
		}
    }

    Picture(const Picture &other) :
	    desc{ other.desc },
	    pts{other.pts},
	    shared{ other.shared }
    {

    }

    Picture &operator=(const Picture &other)
    {
        if (this != &other)
        {
			ReleaseSharedData();
			desc   = other.desc;
			pts    = other.pts;
			shared = other.shared;
        }

        return *this;
    }

    void ReleaseSharedData()
    {
		if (shared)
        {
			if (shared->UnRef() == 0)
			{
				MemoryResource *memoryResource = shared->memoryResource;
				shared->~SharedPictureData();
                if (memoryResource)
                {
					memoryResource->Release(shared);
                }
			}
			shared._obj = nullptr;
        }
    }

    void Swap(Picture &other)
    {
		std::swap(desc,   other.desc  );
		std::swap(pts,    other.pts   );
		std::swap(shared, other.shared);
    }

    template <class T>
    void SetProperty(const T &v)
    {
        if (IsPrimitiveOf<ColorSpace, T>())
        {
            shared->colorSpace = v;
        }
    }

    template <class T>
    const T &GetProperty() const
    {
        if constexpr (IsPrimitiveOf<ColorSpace, T>())
        {
			return shared->colorSpace;
        }
    }

    bool Available() const
    {
        return !!shared;
    }

    operator bool() const
    {
        return !!shared;
    }

    uint8_t*&operator[](size_t index)
    {
        return shared->data[index];
    }

    template <class T = uint8_t>
    auto operator[](size_t index) const
    {
        return (T *)shared->data[index];
    }

    template <class T>
    void Connect(T &&destory)
    {
        shared->destory = destory;
    }

    uint8_t *Data() const
    {
        return shared->data[0];
    }

    uint8_t *GetData() const
    {
		return shared->data[0];
    }

    int GetLineSize(size_t index) const
    {
		return shared->linesize[index];
    }

    void Reset(const void *ptr) const
    {
        shared->data[0] = (uint8_t *)ptr;
    }

    const Format &GetFormat() const
    {
		return desc.format;
    }

    const uint32_t &GetWidth() const
    {
		return desc.width;
    }

    const uint32_t &GetHeight() const
	{
		return desc.height;
	}

    Description desc;

    TimeStamp pts;

    Ref<SharedPictureData> shared;
};

namespace Interface
{

class Codec : public IObject
{
public:
    Codec()
    {

    }

    Codec(Format format)
    {
        picture.desc.format = format;
    }

    virtual ~Codec()
    {

    }

    virtual CodecError Decode(const std::vector<uint8_t> &buffer)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Decode(const CodedFrame &codedFrame)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError DecodeAsync(const std::vector<uint8_t> &buffer)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Parse(const std::vector<uint8_t> &buffer)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Encode(const std::string &filepath, const Picture &picture)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Decode()
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual uint8_t *Data() const
    {
        return picture.Data();
    }

    virtual void Swap(void *ptr)
    {
        ptr = nullptr;
    }

    virtual Picture GetPicture() const
    {
        return picture;
    }

    virtual void Flush()
    {
        picture = Picture{};
    }

protected:
    Picture picture;
};

}

class VideoCodec : public Interface::Codec
{
public:
    template <class T>
    T *GetAddress()
    {
        if (IsPrimitiveOf<T, Animator>())
        {
            return &animator;
        }
        return nullptr;
    }

    virtual CodecError SetCodecContext(Anonymous anonymous)
    {
		(void)anonymous;
		return CodecError::NotImplement;
    }

protected:
    Animator animator;
};

}

using Picture = Vision::Picture;

}
