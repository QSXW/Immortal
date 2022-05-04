#pragma once

#include "Format.h"
#include "Interface/IObject.h"
#include "Media/Types.h"
#include "Media/Common/Error.h"
#include "Media/Common/Animator.h"

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

    std::vector<uint8_t> buffer;
};

using TimeStamp = uint64_t;

struct Picture
{
    Picture() :
        desc{},
        data{},
        pts{}
    {

    }

    bool Available() const
    {
        return !!data;
    }

    Description desc;
    std::shared_ptr<uint8_t> data;
    TimeStamp pts;
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
        desc.format = format;
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

    virtual CodecError Decode()
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual uint8_t *Data() const
    {
        return nullptr;
    }

    virtual void Swap(void *ptr)
    {
        ptr = nullptr;
    }

    virtual Picture GetPicture() const
    {
        return Picture{ };
    }

    const Description &Desc() const
    {
        return desc;
    }

protected:
    Type type{ Type::Unspecifed };

    Description desc;
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
