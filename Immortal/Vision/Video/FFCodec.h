#pragma once

#include "Core.h"
#include "Config.h"
#include "Vision/Codec.h"
#include "Vision/Common/Animator.h"
#include "Memory/MemoryResource.h"
#include "Graphics/LightGraphics.h"
#include "Math/Math.h"

struct AVStream;
struct AVFrame;
struct AVCodec;
struct AVBufferRef;
struct AVCodecContext;
struct AVCodecParameters;
struct SwrContext;
struct SwsContext;
struct AVAudioFifo;
struct AVSubtitle;
namespace Immortal
{
namespace Vision
{

#if HAVE_FFMPEG
class AudioFifo
{
public:
	AudioFifo();

    ~AudioFifo();

    int Allocate(int sampleFormat, int channels, int numSamples = 1024);

	int Allocate(Format format, int channels, int numSamples = 1024);

    int Enqueue(uint8_t **convertedInputSamples, int frameSize);

    int Read(void *const *data, int numSamples) const;

    int Size() const;

public:
    operator bool() const
    {
		return !!handle;
    }

protected:
	AVAudioFifo *handle;
};
#endif

enum class ChannelLayout
{
    Mono,
    Stereo,
    _2Point1,
    _2_1,
    Surround,
    _3Point1,
    _4Point0,
    _4Point1,
    _2_2,
    Quad,
    _5Point0,
    _5Point1,
    _5Point0Back,
    _5Point1Back,
    _6Point0,
    _6Point0Front,
    _3Point1Point2,
    Hexagonal,
    _6Point1,
    _6Point1Back,
    _6Point1Front,
    _7Point0,
    _7Point0Front,
    _7Point1,
    _7Point1Wide,
    _7Point1WideBack,
    _5Point1Point2Back,
    Octagonal,
    Cube,
    _5Point1Point4Back,
    _7Point1Point2,
    _7Point1Point4Back,
    _7Point2Point3,
    _9Point1Point4Back,
    Hexadecagonal,
    StereoDownmix,
    _22Point2,
};

struct AudioFormatSpec
{
	Format format;
	ChannelLayout layout;
	int sampleRate;
	int numChannel;
};

ChannelLayout GetLayoutFromMask(uint64_t mask);

class SampleConverter : public IClass
{
public:
	SampleConverter();

    ~SampleConverter();

    void Release();

    bool SetOptions(const AudioFormatSpec &outputFormat, const AudioFormatSpec &inputFormat);

    int RescaleRound(int numSamples);

    bool Convert(Picture &out, const Picture &input);

    operator bool() const;

public:
    const AudioFormatSpec &GetOutputFormat() const
    {
		return outputFormat;
    }

private:
#if HAVE_FFMPEG
	SwrContext *handle;
#endif

    AudioFormatSpec inputFormat;

    AudioFormatSpec outputFormat;

    int outFormatSize;

    int inputFormatSize;
};

class IMMORTAL_API FFCodec : public VideoCodec
{
#if HAVE_FFMPEG
public:
	FFCodec(int sampleRate = 0, const char *name = "FFmpegDecoder");

    FFCodec(const EncodeInfo &encodeInfo);

    virtual ~FFCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame) override;
    
    virtual CodecError GetPicture(Picture &picture) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;

    virtual CodedFrame GetCodedFrame() const override;

    virtual void Flush() override;

    virtual void *GetProperty(PropertyType type) const override;

    virtual CodecError SetCodecContext(Anonymous anonymous) override;

    CodecError SendAudioFifo();

    void FlushAudioFifo();

    CodecError CreateHardwareAccelerateDevice(const AVCodec *codec);

    CodecError InitializeDecoder(int codecId, const AVStream *stream = nullptr);

    Rational GetFramerate() const;

    Rational GetTimebase() const;

    AudioFormatSpec GetAudioFormat() const;

public:
    void SetPreference(DecodingPreference value)
    {
		preference = value;
    }

    AVCodecContext *GetHandle() const
    {
        return handle;
    }

    int GetSampleRate() const
    {
		return sampleRate;
    }

    void SetSampleRate(int value)
    {
		sampleRate = value;
    }
    
protected:
    CodecError RescaleAudioSamples(int numOutSamples, uint8_t *const *out, int outSampleRate, int numInSamples, const uint8_t *const *in, int inSampleRate, int dataSize);

    CodecError EncodeFrame(AVFrame *frame);

protected:
    AVCodecContext *handle;

    AVBufferRef *device;

    SwrContext *swrContext;

    SwsContext *swsContext;

    int64_t startTimestamp;

    Format format;

    AVFrame *frame;

    PictureMemoryType type;

    MediaType mediaType;

    URef<MemoryResource> memoryResource;

    int hwaccelType;

    int sampleRate;

    int64_t pts;

    DisplayOrientation displayOrientation;

    DecodingPreference preference;

    AudioFifo fifo;

    AVSubtitle *subtitle;

    uint8_t **rescaledSamples = {};

    ColorSpace colorSpace = ColorSpace::BT709;

    int numRescaledSamples = 0;

    bool isEncoder = false;
#endif // HAVE_FFMPEG
};

}
}
