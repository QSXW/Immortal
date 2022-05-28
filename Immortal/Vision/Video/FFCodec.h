#pragma once

#include "Core.h"
#include "Config.h"
#include "Vision/Interface/Codec.h"
#include "Vision/Common/Animator.h"

struct AVFrame;
struct AVCodec;
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

    virtual CodecError Decode(const CodedFrame &codedFrame);

    virtual uint8_t *Data() const;

    virtual Picture GetPicture() const;

public:
    void SetCodecContext(const AVCodec *codec, const AVCodecParameters *parameters);

protected:
    AVCodecContext *handle;

    AVFrame *frame;
#endif // HAVE_FFMPEG
};

}
}
