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
    virtual ~Demuxer()
    {}

    virtual CodecError Open(const std::string &filepath, VideoCodec *codec)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Read(CodedFrame *codedFrame)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual const std::string &GetSource() const = 0;
};

}
}
}
