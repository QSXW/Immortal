#pragma once

#include "Shared/IObject.h"
#include "Types.h"
#include "Picture.h"
#include "CodedFrame.h"
#include "Common/Error.h"
#include "Common/Animator.h"
#include "Math/Math.h"

namespace Immortal
{
namespace Vision
{

namespace Interface
{

class IMMORTAL_API Codec : public IObject
{
public:
	Codec(const char *name = {}) :
	    mediaType{MediaType::Video},
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

    virtual CodecError GetPicture(Picture &picture)
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

    virtual CodedFrame GetCodedFrame() const
    {
		return {};
    }

    virtual Picture GetPicture() const
    {
        return picture;
    }

    virtual CodecError DecodeHeader(CodedFrame &codedFrame, EncodeInfo &info)
    {
		return CodecError::FailedToCallDecoder;
    }

    virtual void Flush()
    {
        picture = Picture{};
    }

    virtual void *GetProperty(PropertyType property) const
    {
		return nullptr;
    }

    template <class T>
    const T *GetProperty() const
    {
		return (const T *)GetProperty(T::Type);
    }

    MediaType GetMediaType() const
    {
		return mediaType;
    }

protected:
    Picture picture;

    MediaType mediaType;
};

class IMMORTAL_API VideoCodec : public Interface::Codec
{
public:
	VideoCodec(const char *name = {}) :
	    Interface::Codec{name},
	    animator{}
    {

    }

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
