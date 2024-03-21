#include "TurboJpegCodec.h"

#if HAVE_TURBOJPEG
#include <turbojpeg.h>
#endif

namespace Immortal
{
namespace Vision
{

#if HAVE_TURBOJPEG
TurboJpegCodec::TurboJpegCodec() :
    Super{}
{

}

TurboJpegCodec::~TurboJpegCodec()
{

}

CodecError TurboJpegCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.GetBuffer();

    return Decode(buffer.data(), buffer.size());
}

CodecError TurboJpegCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
    return CodecError::NotImplement;
}
#endif

CodecError TurboJpegCodec::Decode(const uint8_t *data, size_t size)
{
#if HAVE_TURBOJPEG
    tjhandle handle = {};

    handle = tjInitDecompress();

    int width, height, subsample, colorSpace;
	tjDecompressHeader3(handle, data, size, &width, &height, &subsample, &colorSpace);

    tjscalingfactor scalingFactor = {
        .num   = 1,
        .denom = 4
    };

    picture = Picture{ width, height, Format::RGBA8, true };
	auto pitch = picture.GetWidth() * 4;

	tjDecompress2(handle, data, size, picture.GetData(), width, pitch, height, TJPF_RGBA, 0);
	tjDestroy(handle);

    return CodecError::Success;
#else
	return CodecError::NotImplement;
#endif
}

}
}
