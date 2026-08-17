#pragma once

#include "Vision/Codec.h"
#include "String/IString.h"

namespace Immortal
{
namespace Vision
{

class GprCodec : public Interface::Codec
{
public:
    explicit GprCodec(
        Format outputFormat = Format::RGBA8,
        const String &sourcePath = {});

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError DecodeHeader(CodedFrame &codedFrame, CodecInfo &info) override;

    CodecError DecodeThumbnail(const CodedFrame &codedFrame, uint32_t requestedWidth);

private:
    Format format;

    String sourcePath;
};

}
}
