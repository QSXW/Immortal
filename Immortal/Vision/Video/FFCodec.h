#pragma once

#include "Core.h"
#include "Config.h"
#include "Vision/Codec.h"
#include "Vision/Common/Animator.h"
#include "Memory/MemoryResource.h"
#include "Graphics/LightGraphics.h"

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
    FFCodec();

    virtual ~FFCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual Picture GetPicture() const override;

    virtual void Flush() override;

public:
	virtual CodecError SetCodecContext(Anonymous anonymous) override;

protected:
    AVCodecContext *handle;

    AVBufferRef *device;

    int64_t startTimestamp;

    Format format;

    AVFrame *frame;

    PictureMemoryType type;

    URef<MemoryResource> memoryResource;
    
    int hwaccelType;
#endif // HAVE_FFMPEG
};

}
}
