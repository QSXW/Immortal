#pragma once

#include "Media/Interface/MFXCodec.h"

#if HAVE_MFX
#include <mfx/mfxvideo++.h>
#endif

namespace Immortal
{
namespace Vision
{

class IMMORTAL_API MFXJpegCodec : public MFXCodec
{
#if HAVE_MFX
public:
    using Super = MFXCodec;

public:
    MFXJpegCodec();

    virtual ~MFXJpegCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual uint8_t *Data() const override;

protected:
    MonoRef<uint8_t> data;
#endif
};

}
}
