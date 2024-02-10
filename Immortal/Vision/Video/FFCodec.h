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

class IMMORTAL_API FFCodec : public VideoCodec
{
#if HAVE_FFMPEG
public:
    FFCodec(int sampleRate = 0);

    virtual ~FFCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual Picture GetPicture() const override;

    virtual void Flush() override;

    virtual CodecError SetCodecContext(Anonymous anonymous) override;

    CodecError InitializeDecoder(int codecId, const AVStream *stream = nullptr);

public:
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
#endif // HAVE_FFMPEG
};

}
}
