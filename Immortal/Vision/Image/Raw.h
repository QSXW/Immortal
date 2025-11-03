#pragma once

#include "Vision/Codec.h"

class LibRaw;
namespace Immortal
{
namespace Vision
{

struct RawParams
{
	float black[4];
	float scale[4];
};

class RawCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
	RawCodec(Format outputFormat = Format::BayerLayerRGGB, int scale = 1);

    virtual ~RawCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError DecodeHeader(CodedFrame &codedFrame, EncodeInfo &Info) override;

    void GetParams(RawParams *pParams);

    void GetProjectionMatrix(float matrix[4][4]);

    void GetCurve(float *curve);

    DisplayOrientation GetDisplayOrientation();

    Picture DecodeThumbnail(const CodedFrame &codedFrame);

protected:
    Format format;

    int scale;

    std::shared_ptr<LibRaw> processor;
};

}
}
