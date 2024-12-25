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

	int Allocate(AVCodecContext *codecContext, int numSamples = 1024);

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

class IMMORTAL_API FFCodec : public VideoCodec
{
#if HAVE_FFMPEG
public:

    FFCodec(int sampleRate = 0);

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

    int numRescaledSamples = 0;

    bool isEncoder = false;
#endif // HAVE_FFMPEG
};

}
}
