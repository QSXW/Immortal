#pragma once

#include "Shared/IObject.h"
#include "Types.h"
#include "Picture.h"
#include "CodedFrame.h"
#include "Common/Error.h"
#include "Common/Animator.h"

namespace Immortal
{
namespace Vision
{

enum class PropertyType
{
	DisplayOrientation,
};

struct DisplayOrientation
{
	static constexpr PropertyType Type = PropertyType::DisplayOrientation;
	int hflip;
	int anticlockwiseRotation;
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

    virtual void *GetProperty(PropertyType property) const
    {
		return nullptr;
    }

    template <class T>
    const T *GetProperty() const
    {
		return (const T *)GetProperty(T::Type);
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
