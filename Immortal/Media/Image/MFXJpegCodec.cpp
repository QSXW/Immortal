#include "MFXJpegCodec.h"
#include "Media/Processing/ColorSpace.h"

#if HAVE_MFX

#include <mfx/mfxjpeg.h>

namespace Immortal
{
namespace Vision
{

MFXJpegCodec::MFXJpegCodec() :
    Super{}
{
    videoParam.mfx.CodecId                = MFX_CODEC_JPEG;
    videoParam.mfx.FrameInfo.FourCC       = MFX_FOURCC_NV12;
    videoParam.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    videoParam.IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
}

MFXJpegCodec::~MFXJpegCodec()
{

}

CodecError MFXJpegCodec::Decode(const CodedFrame &codedFrame)
{
    mfxStatus status;
    mfxFrameSurface1 *pOutSurface{};

    bitstream = BitStreamReference{ codedFrame.buffer };
    if ((status = CheckAdapterSupported()) != MFX_ERR_NONE)
    {
        return CodecError::ExternalFailed;
    }
  
    if ((status = handle->DecodeHeader(&bitstream, &videoParam)) != MFX_ERR_NONE)
    {
        LOG::ERR("Failed to decode header: {}", status);
        return CodecError::ExternalFailed;
    }

    bitstream.DataFlag |= MFX_BITSTREAM_COMPLETE_FRAME;

    if ((status = handle->Init(&videoParam)) != MFX_ERR_NONE)
    {
        LOG::ERR("Failed to init decoder: {}", status);
        return CodecError::ExternalFailed;
    }

    desc.format  = Format::RGBA8;
    desc.width   = videoParam.mfx.FrameInfo.CropW;
    desc.height  = videoParam.mfx.FrameInfo.CropH;
    MonoRef<uint8_t> temp = new uint8_t[desc.Size()];

    surface.Info = videoParam.mfx.FrameInfo;
    surface.Data.PitchLow = surface.Info.Width;
    surface.Data.Y  = &temp[0];
    surface.Data.UV = &temp[surface.Data.PitchLow * surface.Info.Height];

    data = new uint8_t[desc.Size()];

    if ((status = handle->DecodeFrameAsync(&bitstream, &surface, &pOutSurface, &syncPoint)) != MFX_ERR_NONE)
    {
        LOG::ERR("Failed to decode frane(Aysnc): {}", status);
        return CodecError::ExternalFailed;
    }

    do
    {
        status = session->SyncOperation(syncPoint, 1000);
    } while (status == MFX_WRN_IN_EXECUTION);

    handle->Close();

    ColorSpace::Vector<uint8_t> dst{};
    dst.r = data;

    ColorSpace::Vector<uint8_t> src{};
    src.x = pOutSurface->Data.Y;
    src.y = pOutSurface->Data.UV;

    ColorSpace::NV12ToRGBA8(dst, src, desc.width, desc.height, pOutSurface->Data.PitchLow, pOutSurface->Data.PitchLow);

    return CodecError::Succeed;
}

uint8_t *MFXJpegCodec::Data() const
{
    return data;
}

}
}

#endif
