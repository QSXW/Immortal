#pragma once

#include <cstdint>

#include "Vision/Codec.h"


namespace Immortal
{
namespace Vision
{

class TurboJpegCodec : public Interface::Codec, public IClass
{
public:
    using Super = Interface::Codec;

#if HAVE_TURBOJPEG
public:
	TurboJpegCodec(bool isOutputYUV = false);

    TurboJpegCodec(const ImageEncodeInfo &info);

    virtual ~TurboJpegCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;
#endif

    CodecError Decode(const uint8_t *data, size_t size);

    void SetScale(int numerator, int denominator);

    void SetQuality(int value);

    void SetDesireSize(int value);

public:
	ImageEncodeInfo info;

protected:
	bool isOutputYUV;

    int numerator;

    int denominator;

    int desireSize;
};

}
}
