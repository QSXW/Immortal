#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <string>
#include <memory>

#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

enum BMPFormat
{
    Unknown,
	ABGR,
	XBGR,
	BGRA,
	BGRX,
	ARGB,
	XRGB,
	RGBA,
	RGBX,
	BGR24,
	RGB555,
	RGB565,
	RGB444,
    PAL8,
    GRAY8,
};

class BMPCodec : public Interface::Codec, public IClass
{
public:
    using Super = Interface::Codec;

public:
	BMPCodec();

    virtual ~BMPCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame);

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;

protected:
	BMPFormat GetFormat(int compression, const uint32_t *rgb, int depth, int alpha, bool hasInfo);
};

}
}
