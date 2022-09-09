#pragma once

#include "Config.h"
#include "Vision/Interface/Codec.h"
#include "Vision/Interface/Demuxer.h"
#include "FileSystem/Stream.h"

struct AVPacket;
namespace Immortal
{
namespace Vision
{

class FormatContext;
class IMMORTAL_API FFDemuxer : public Interface::Demuxer
{
#if HAVE_FFMPEG
public:
    FFDemuxer();

    ~FFDemuxer();

    virtual CodecError Open(const std::string &filepath, VideoCodec *codec) override;

    virtual CodecError Read(CodedFrame *codedFrame) override;

private:
    Ref<FormatContext> formatContext;

    AVPacket *packet;
#endif

public:
    virtual const std::string &GetSource() const override;

private:
    std::string filepath;

};

}
}
