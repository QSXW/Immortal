#pragma once

#include "Config.h"
#include "Vision/Interface/Codec.h"
#include "Vision/Interface/Demuxer.h"
#include "FileSystem/Stream.h"



struct AVCodec;
struct AVStream;
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
	struct Params
	{
		const AVStream *stream;
	};

public:
    FFDemuxer();

    ~FFDemuxer();

    virtual CodecError Open(const std::string &filepath, VideoCodec *codec, VideoCodec *audioCodec = nullptr) override;

    virtual CodecError Read(CodedFrame *codedFrame) override;

    void Destory();

private:
    Ref<FormatContext> formatContext;
#endif

public:
    virtual const std::string &GetSource() const override;

private:
    std::string filepath;

};

}
}
