#pragma once

#include "Format.h"
#include "Interface/IObject.h"
#include "Vision/Types.h"
#include "Vision/Common/Error.h"
#include "Vision/Common/Animator.h"

namespace Immortal
{
namespace Vision
{

struct CodedFrame
{
    int64_t timestamp;
    int64_t duration;
    int64_t offset;

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

    std::vector<uint8_t> buffer;
};

using TimeStamp = uint64_t;

struct PictureExtension : public IObject
{
    ColorSpace colorSpace;
};

/** Shared Picture Data
 *
 *  This struct only can used as a shared pointer
 *  Constructor once and Deconstructor once
 */
struct SharedPictureData : IObject
{
    SharedPictureData() :
        data{},
        linesize{},
        destory{}
    {

    }

    SharedPictureData(int width, int height, Format format) :
        data{},
        linesize{},
        destory{}
    {
        data[0] = new uint8_t[width * height * format.Size()];
    }

    ~SharedPictureData()
    {
        destory ? destory() : delete[]data[0];
    }

    uint8_t *data[8];
    int linesize[8];
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

    Picture(int width, int height, Format format, bool allocated = true) :
        desc{ uint32_t(width), uint32_t(height), format },
        pts{}
    {
		if (desc.format.IsType(Format::YUV))
        {
            extension = new PictureExtension;
        }
        if (allocated)
        {
            shared = new SharedPictureData{ width, height, format };
        }
        else
        {
            shared = new SharedPictureData{};
        }
    }

    template <class T>
    void SetProperty(const T &v)
    {
        if (IsPrimitiveOf<ColorSpace, T>())
        {
            extension.DeRef<PictureExtension>().colorSpace = v;
        }    
    }

    template <class T>
    const T &GetProperty() const
    {
        if constexpr (IsPrimitiveOf<ColorSpace, T>())
        {
            return extension.DeRef<PictureExtension>().colorSpace;
        }
    }

    bool Available() const
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

    void Reset(const void *ptr) const
    {
        shared->data[0] = (uint8_t *)ptr;
    }

    Description desc;

    Ref<SharedPictureData> shared;

    TimeStamp pts;

    Ref<IObject> extension;
};

namespace Interface
{

class Codec : public IObject
{
public:
    Codec()
    {

    }

    Codec(Type type, Format format) :
        type{ type }
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

protected:
    Type type{ Type::Unspecifed };

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

protected:
    Animator animator;
};

}
}
