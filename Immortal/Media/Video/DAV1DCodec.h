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

    virtual CodecError Decode(CodedFrame *codedFrame);

    virtual uint8_t *Data() const;

private:
    void CheckVersion();

protected:
    Dav1dContext *handle;

    MonoRef<uint8_t> data;

    const char *version;
#endif
};

}
}
