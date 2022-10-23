#include "MFXJpegCodec.h"
#include "Vision/Processing/ColorSpace.h"

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

    picture = Picture{
        videoParam.mfx.FrameInfo.CropW,
        videoParam.mfx.FrameInfo.CropH,
        Format::RGBA8
    };

    URef<uint8_t> temp = new uint8_t[picture.desc.Size()];

    surface.Info = videoParam.mfx.FrameInfo;
    surface.Data.PitchLow = surface.Info.Width;
    surface.Data.Y  = &temp[0];
    surface.Data.UV = &temp[surface.Data.PitchLow * surface.Info.Height];

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

    CVector<uint8_t> dst{};
    dst.r = picture.Data();

    CVector<uint8_t> src{};
    src.x = pOutSurface->Data.Y;
    src.y = pOutSurface->Data.UV;
    src.linesize[0] = pOutSurface->Data.PitchLow;
    src.linesize[1] = pOutSurface->Data.PitchLow;

    NV12ToRGBA8(dst, src, picture.desc.width, picture.desc.height);

    return CodecError::Succeed;
}

}
}

#endif
