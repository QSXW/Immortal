#pragma once

#include "Core.h"
#include "Config.h"
#include "Media/Interface/Codec.h"
#include "Media/Common/Animator.h"

struct Dav1dContext;
namespace Immortal
{
namespace Vision
{

class IMMORTAL_API DAV1DCodec : public VideoCodec
{
#if HAVE_DAV1D
public:
    DAV1DCodec();

    virtual ~DAV1DCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame);

    virtual uint8_t *Data() const;

    virtual Picture GetPicture() const;

private:
    void CheckVersion();

protected:
    Dav1dContext *handle;

    Picture picture;

    const char *version;
#endif
};

}
}
