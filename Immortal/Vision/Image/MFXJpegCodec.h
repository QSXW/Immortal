#pragma once

#include "Vision/Interface/MFXCodec.h"

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
#endif
};

}
}
