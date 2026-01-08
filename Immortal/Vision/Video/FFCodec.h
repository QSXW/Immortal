#pragma once

#include "Core.h"
#include "Config.h"
#include "Vision/Types.h"
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

int64_t RationalRescale(int64_t a, Rational bq, Rational cq);

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

ChannelLayout GetLayoutFromMask(uint64_t mask);

class SampleConverter : public IClass, public IObject
{
public:
	SampleConverter();

    ~SampleConverter();

    void Release();

    bool SetOptions(const AudioFormatSpec &outputFormat, const AudioFormatSpec &inputFormat);

    int RescaleRound(int numSamples);

    bool Convert(Picture &out, const Picture &input);

    Picture GetRemainingSamples();

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

class IMMORTAL_API FFCodec : public VideoCodec, public IClass
{
#if HAVE_FFMPEG
public:
	FFCodec(const char *name = "FFmpegDecoder");

    FFCodec(CodecId codecId);

    FFCodec(const CodecInfo &encodeInfo);

    virtual ~FFCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError GetPicture(Picture &picture) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;

    virtual CodedFrame GetCodedFrame() const override;

    virtual void Flush() override;

    virtual void *GetProperty(PropertyType type) const override;

    virtual CodecError OpenDecoder(CodecInfo &info) override;

    Picture ScaleToSupportFormat(AVFrame *frame);

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

    operator bool() const
    {
		return !!handle;
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

    URef<MemoryResource> memoryResource;

    int hwaccelType;

    int64_t pts;

    DisplayOrientation displayOrientation;

    DecodingPreference preference;

    AudioFifo fifo;

    AVSubtitle *subtitle;

    uint8_t **rescaledSamples = {};

    ColorSpace colorSpace = ColorSpace::BT709;

    ColorTransferCharacteristic transferCharacteristic = ColorTransferCharacteristic::Unspecified;

    int numRescaledSamples = 0;

    bool isEncoder = false;
#endif // HAVE_FFMPEG
};

class FFImageCodec : public Interface::Codec
{
public:
	using Super = Interface::Codec;

public:
	FFImageCodec(CodecId codecId);

	virtual ~FFImageCodec() override;

	virtual CodecError Decode(const CodedFrame &codedFrame) override;

	virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;

private:
    CodecId codecId;
};

template <CodecId C>
class TFFImageCodec : public FFImageCodec
{
public:
    TFFImageCodec() :
        FFImageCodec{ C }
    {

    }
};

using FFTiffCodec   = TFFImageCodec<CodecId::TIFF>;
using FFPngCodec    = TFFImageCodec<CodecId::PNG>;
using FFWebpCodec   = TFFImageCodec<CodecId::WEBP>;
using FFJpegxlCodec = TFFImageCodec<CodecId::JPEGXL>;

}
}
