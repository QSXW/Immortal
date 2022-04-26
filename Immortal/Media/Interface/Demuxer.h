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
namespace Interface
{

class Demuxer : public IObject
{
public:
    virtual CodecError Open(const std::string &filepath, VideoCodec *codec)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Read(CodedFrame *codedFrame)
    {
        return CodecError::FailedToCallDecoder;
    }
};

}
}
}
