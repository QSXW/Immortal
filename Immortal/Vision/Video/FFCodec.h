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

int64_t RationalRescaleRound(int64_t a, int64_t b, int64_t c, int rnd = 5);

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

    void Reset();

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

class Scaler
{
public:
	Scaler();

    ~Scaler();

    void Init(uint32_t srcW, uint32_t srcH, int srcFormat, uint32_t dstW, uint32_t dstH, int dstFormat, int flags);

    void Init(uint32_t srcW, uint32_t srcH, Format srcFormat, uint32_t dstW, uint32_t dstH, Format dstFormat, Filter filter);

    CodecError Scale(const uint8_t *const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t *const dst[], const int dstStride[]);

    CodecError Scale(Picture &dst, const Picture &src);

    operator bool() const;

protected:
#if HAVE_FFMPEG
	SwsContext *handle;
#endif
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

    CodecError DecodeSubtitleCue(const CodedFrame &codedFrame, SubtitleCue &cue);

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

    CodecError ReopenDecoderAsSoftware();

    Rational GetFramerate() const;

    Rational GetTimebase() const;

    AudioFormatSpec GetAudioFormat() const;

    const String &LastError() const
    {
        return lastError;
    }

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

    Scaler scaler;

    int64_t startTimestamp;

    Format format;

    AVFrame *frame;

    PictureMemoryType type;

    URef<MemoryResource> memoryResource;

    int hwaccelType;

    int64_t pts;

    DisplayOrientation displayOrientation;

    Rational sampleAspectRatio;

    DecodingPreference preference;

    AudioFifo fifo;

    AVSubtitle *subtitle;

    uint8_t **rescaledSamples = {};

    ColorSpace colorSpace = ColorSpace::BT709;

    ColorTransferCharacteristic transferCharacteristic = ColorTransferCharacteristic::Unspecified;

    int numRescaledSamples = 0;

    bool isEncoder = false;

    int64_t audioNextPtsSync  = INT64_MIN;
    int64_t audioNextPtsTbNum = 0;
    int64_t audioNextPtsTbDen = 1;

    const AVStream *decoderStream = nullptr;
    int decoderCodecId = 0;
    bool hardwareFallbackAttempted = false;

    mutable String lastError;
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

using FFTiffCodec      = TFFImageCodec<CodecId::TIFF>;
using FFPngCodec       = TFFImageCodec<CodecId::PNG>;
using FFWebpCodec      = TFFImageCodec<CodecId::WEBP>;
using FFJpegxlCodec    = TFFImageCodec<CodecId::JPEGXL>;
using FFMjpegCodec     = TFFImageCodec<CodecId::MJPEG>;
using FFJpeg2000Codec  = TFFImageCodec<CodecId::JPEG2000>;
using FFDPXCodec       = TFFImageCodec<CodecId::DPX>;
using FFTargaCodec     = TFFImageCodec<CodecId::TARGA>;
using FFPCXCodec       = TFFImageCodec<CodecId::PCX>;
using FFExrCodec       = TFFImageCodec<CodecId::EXR>;
using FFSgiCodec       = TFFImageCodec<CodecId::SGI>;
using FFSunRasterCodec = TFFImageCodec<CodecId::SUNRASTER>;
using FFJpegLsCodec    = TFFImageCodec<CodecId::JPEGLS>;
using FFFitsCodec      = TFFImageCodec<CodecId::FITS>;
using FFIffIlbmCodec   = TFFImageCodec<CodecId::IFF_ILBM>;
using FFXbmCodec       = TFFImageCodec<CodecId::XBM_IMAGE>;
using FFXfaceCodec     = TFFImageCodec<CodecId::XFACE>;
using FFQdrawCodec     = TFFImageCodec<CodecId::QDRAW>;

}
}
