#pragma once

#include "Shared/IObject.h"
#include "Codec.h"
#include "Vision/Types.h"
#include "Vision/Common/Error.h"
#include "Vision/Common/Animator.h"
#include "String/IString.h"

namespace Immortal
{

namespace Vision
{

class IMMORTAL_API Demuxer : public IObject
{
public:
    virtual ~Demuxer() = default;

    /**
     * @brief Open a file
     */
    virtual CodecError Open(const String &filepath, VideoCodec *codec, VideoCodec *audioCodec = nullptr, VideoCodec *subtitleCodec = nullptr)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Open(const String &filepath, Codec *videoCodec, Codec *audioCodec, Codec *subtitleCodec, const std::vector<MediaType> &streams)
    {
		return CodecError::FailedToCallDecoder;
    }

    /**
     * @brief Close the demuxer,
     *
     */
    virtual void Close()
    {

    }

    /**
     * @brief Read a coded frame or data block
     */
    virtual CodecError Read(CodedFrame *codedFrame)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Write(const CodedFrame &codedFrame, int stream = 0)
    {
		return CodecError::FailedToCallDecoder;
    }

    virtual CodecError Seek(MediaType type, double seconds, int64_t min, int64_t max)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual const String &GetSource() const = 0;
};

}

using Demuxer = Vision::Demuxer;

}
