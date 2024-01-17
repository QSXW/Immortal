#pragma once

#include "Codec.h"
#include "Demuxer.h"
#include "FileSystem/Stream.h"

namespace Immortal
{
namespace Vision
{

class IMMORTAL_API IVFDemuxer : public Demuxer
{
public:
    struct Header
    {
        size_t   size;
        uint64_t offset;
        uint64_t timestamp;
    };

public:
    IVFDemuxer();

    virtual CodecError Open(const String &filepath, VideoCodec *codec, VideoCodec* audioCodec = nullptr) override;

    virtual CodecError Read(CodedFrame *codedFrame) override;

private:
    Header ReadHeader();

protected:
    Stream stream;
};

}
}
