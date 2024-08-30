#pragma once

#include "Core.h"
#include "Config.h"
#include "Vision/Codec.h"
#include "Vision/Common/Animator.h"
#include "Memory/MemoryResource.h"
#include "Graphics/LightGraphics.h"

struct AVStream;
struct AVFrame;
struct AVCodec;
struct AVBufferRef;
struct AVCodecContext;
struct AVCodecParameters;
namespace Immortal
{
namespace Vision
{

enum class DecodingPreference
{
    Auto,
	Software,
    Hardware,
};

class IMMORTAL_API FFCodec : public VideoCodec
{
#if HAVE_FFMPEG
public:
    FFCodec(int sampleRate = 0);

    virtual ~FFCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual Picture GetPicture() const override;

    virtual void Flush() override;

    virtual void *GetProperty(PropertyType type) const override;

    virtual CodecError SetCodecContext(Anonymous anonymous) override;

    CodecError CreateHardwareAccelerateDevice(const AVCodec *codec);

    CodecError InitializeDecoder(int codecId, const AVStream *stream = nullptr);

public:
    void SetPreference(DecodingPreference value)
    {
		preference = value;
    }

    AVCodecContext *GetHandle() const
    {
        return handle;
    }

protected:
    AVCodecContext *handle;

    AVBufferRef *device;

    int64_t startTimestamp;

    Format format;

    AVFrame *frame;

    PictureMemoryType type;

    URef<MemoryResource> memoryResource;

    int hwaccelType;

    int sampleRate;

    DisplayOrientation displayOrientation;

    DecodingPreference preference;
#endif // HAVE_FFMPEG
};

}
}
