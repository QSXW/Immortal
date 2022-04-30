#pragma once

#include "Core.h"
#include "Config.h"
#include "Media/Interface/Codec.h"
#include "Media/Common/Animator.h"

#if HAVE_MFX
#include <mfx/mfxadapter.h>
#include <mfx/mfxvideo++.h>
#endif

namespace Immortal
{
namespace Vision
{

class IMMORTAL_API MFXCodec : public VideoCodec
{
#if HAVE_MFX
public:
    struct BitStreamReference : public mfxBitstream
    {
        BitStreamReference()
        {

        }

        BitStreamReference(const std::vector<uint8_t> &buffer) :
            mfxBitstream{}
        {
            Data       = (decltype(Data))buffer.data();
            DataLength = buffer.size();
            MaxLength  = DataLength;
        }
    };

    struct FrameSurface : public mfxFrameSurface1
    {
       
    };

public:
    MFXCodec();

    virtual ~MFXCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame);

protected:
    mfxStatus CheckAdapterSupported();

protected:
    MonoRef<MFXVideoDECODE> handle;
    MonoRef<MFXVideoSession> session;
    mfxVideoParam videoParam;
    BitStreamReference bitstream;
    FrameSurface surface;
    mfxSyncPoint syncPoint;
    mfxVersion version;
    mfxIMPL impl;
#endif
};

}
}
