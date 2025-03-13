#pragma once

#include "Config.h"
#include "Vision/Codec.h"
#include "Vision/Demuxer.h"
#include "FileSystem/Stream.h"
#include "String/IString.h"

struct AVCodec;
struct AVStream;
struct AVPacket;
struct AVFormatContext;
struct AVIOContext;
struct AVCodecContext;
namespace Immortal
{
namespace Vision
{

class FormatContext;
class IMMORTAL_API FFDemuxer : public Demuxer
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

    virtual CodecError Open(const String &filepath, VideoCodec *codec, VideoCodec *audioCodec = nullptr, VideoCodec *subtitleCodec = nullptr) override;

    virtual CodecError Open(const String &filepath, Codec **pCodec, uint32_t numCodec) override;

    virtual void Close() override;

    virtual CodecError Read(CodedFrame *codedFrame) override;

    virtual CodecError Write(const CodedFrame &codedFrame, int stream = 0) override;

    virtual CodecError Seek(MediaType type, double seconds, int64_t min, int64_t max) override;

    virtual void EnumerateTracks(MediaType mediaType, std::vector<TrackInfo> &tracks);
    
    virtual CodecError SwitchTrack(MediaType mediaType, int index);

    void Destroy();

private:
    Ref<FormatContext> formatContext;

    AVFormatContext *handle;

    AVIOContext *pb;

    AVCodecContext *codecs[4];

    std::vector<AVStream *> streams;
#endif

public:
    virtual const String &GetSource() const override;

protected:
    String filepath;
};

}
}
