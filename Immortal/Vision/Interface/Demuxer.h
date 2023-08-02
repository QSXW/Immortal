#pragma once

#include "Format.h"
#include "Interface/IObject.h"
#include "Vision/Types.h"
#include "Vision/Common/Error.h"
#include "Vision/Common/Animator.h"

#include <string>

namespace Immortal
{
namespace Vision
{
namespace Interface
{

class Demuxer : public IObject
{
public:
    virtual ~Demuxer() {}

    virtual CodecError Open(const std::string &filepath, VideoCodec *codec, VideoCodec *audioCodec = nullptr)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Read(CodedFrame *codedFrame)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Seek(MediaType type, double seconds, int64_t min, int64_t max)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual const std::string &GetSource() const = 0;
};

}
}
}
