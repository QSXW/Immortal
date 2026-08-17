#pragma once

#include "Config.h"
#include "Vision/Codec.h"
#include "Vision/MediaFormat.h"
#include "FileSystem/Stream.h"
#include "String/IString.h"

#include <vector>

struct AVCodec;
struct AVStream;
struct AVPacket;
struct AVDictionary;
struct AVInputFormat;
struct AVFormatContext;
struct AVIOContext;
struct AVCodecContext;
namespace Immortal
{
namespace Vision
{

class IMMORTAL_API FFFormat : public MediaFormat
{
public:
    struct MetadataProperty
    {
        String name;
        String value;
    };

    struct MetadataSection
    {
        String title;
        std::vector<MetadataProperty> properties;
    };

    using MetadataSections = std::vector<MetadataSection>;

#if HAVE_FFMPEG
public:
	struct Params
	{
		const AVStream *stream;
	};

public:
    FFFormat();

    ~FFFormat();

    virtual CodecError Open(const String &filepath) override;

    virtual CodecError Open(const String &filepath, Codec **pCodec, const CodecInfo *encodeInfos, uint32_t numCodec) override;

    virtual void Close() override;

    virtual CodecError Read(CodedFrame *codedFrame) override;

    virtual CodecError Write(const CodedFrame &codedFrame, int stream = 0) override;

    virtual CodecError Seek(MediaType type, int64_t pts, int64_t min, int64_t max) override;

    virtual CodecError GetStreamInfo(MediaType type, CodecInfo &streamInfo);

    virtual void EnumerateTracks(MediaType mediaType, std::vector<TrackInfo> &tracks);

    virtual CodecError SwitchTrack(MediaType mediaType, int index);

    virtual Animator &GetAnimator(MediaType mediaType);

    double GetMaxFrameDurationForSync() const;
#endif

    const String &LastError() const
    {
        return lastError;
    }

    CodecError GetMetadata(MetadataSections &sections) const;

private:
	void SetAnimator();

    int OpenWithWin32FileIO(const AVInputFormat *inputFormat, AVDictionary **options);

    void CloseCustomIO();

private:
    AVFormatContext *handle;

    AVIOContext *customIO;

    void *customIOOpaque;

    int streamIndex[4];

	std::vector<Animator> animators;

    AVCodecContext *codecs[4];

    std::vector<AVStream *> streams;

public:
    virtual const String &GetSource() const override;

protected:
    String filepath;

    String lastError;
};

}
}
