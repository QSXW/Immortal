#pragma once

#include "Shared/IObject.h"
#include "Vision/Types.h"
#include "Vision/Picture.h"
#include "Vision/Common/Error.h"
#include "Vision/Common/Animator.h"

#include <cstdint>
#include <vector>

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

public:
    std::vector<uint8_t> buffer;

    MediaType Type;
};

namespace Interface
{

class IMMORTAL_API Codec : public IObject
{
public:
    Codec() :
	    picture{}
    {

    }

    virtual ~Codec() = default;

    /**
	 * @brief Decode a coded buffer
	 */
    virtual CodecError Decode(const CodedFrame &codedFrame)
    {
        return CodecError::FailedToCallDecoder;
    }

    /**
     * @brief Encode a picture to the coded data
     */
    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame)
    {
        return CodecError::FailedToCallDecoder;
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

class IMMORTAL_API VideoCodec : public Interface::Codec
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
}

using Codec      = Vision::Interface::Codec;
using VideoCodec = Vision::Interface::VideoCodec;

}
