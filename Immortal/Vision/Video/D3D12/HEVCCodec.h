#pragma once

#include "Config.h"
#include "Core.h"
#include "Vision/Common/Animator.h"
#include "Vision/Interface/Codec.h"
#include "Platform/D3D12/VideoCommon.h"
#include "Vision/Video/HEVC.h"

namespace Immortal
{
namespace Vision
{
namespace D3D12
{

using namespace Immortal::D3D12;

class IMMORTAL_API HEVCCodec : public SuperHEVCCodec
{
#if HAVE_FFMPEG
public:
	HEVCCodec();

	virtual ~HEVCCodec();

	virtual CodecError Decode(const CodedFrame &codedFrame) override;
#endif

protected:
	URef<VideoDecoder> handle;

	URef<VideoDevice> videoDevice;

	URef<VideoDecoderHeap> videoDecoderHeap;
};

}
}
}
