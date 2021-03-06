#pragma once

#include "Vision/Interface/Codec.h"
#include "Vision/Interface/Demuxer.h"
#include "FileSystem/Stream.h"

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

    virtual const std::string &GetSource() const override;

private:
    Header ReadHeader();

protected:
    Stream stream;
};

}
}
