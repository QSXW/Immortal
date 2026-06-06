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
	RawCodec(Format outputFormat = Format::BayerLayerRGGB);

    virtual ~RawCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    void GetParams(RawParams *pParams);

    void GetProjectionMatrix(float matrix[4][4]);

    void GetCurve(float *curve);

    int GetFlipType();

protected:
    Format format;

    std::shared_ptr<LibRaw> processor;
};

}
}
