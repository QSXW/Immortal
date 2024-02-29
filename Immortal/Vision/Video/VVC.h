#pragma once

#include "Core.h"
#include "Codec.h"
#include "Vision/Common/BitTracker.h"
#include "Vision/Common/NetworkAbstractionLayer.h"

namespace Immortal
{
namespace Vision
{

using NAL = NetworkAbstractionLayer;

class IMMORTAL_API VVCCodec : public VideoCodec
{
public:
	virtual CodecError Decode(const CodedFrame &codedFrame) override;

    CodecError Parse(const CodedFrame &codedFrame);

protected:

};

using SuperVVCCodec = VVCCodec;

}
}

