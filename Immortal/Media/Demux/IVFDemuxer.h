#pragma once

#include "Media/Interface/Codec.h"
#include "Media/Interface/Demuxer.h"
#include "Stream.h"

namespace Immortal
{
namespace Vision
{

class IMMORTAL_API IVFDemuxer : public Interface::Demuxer
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

    virtual CodecError Open(const std::string &filepath, VideoCodec *codec);

    virtual CodecError Read(CodedFrame *codedFrame);

private:
    Header ReadHeader();

protected:
    Stream stream;
};

}
}
