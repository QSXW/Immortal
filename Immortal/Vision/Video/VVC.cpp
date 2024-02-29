#include "VVC.h"
#include "VVCParser.h"

#if HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

namespace Immortal
{
namespace Vision
{

CodecError VVCCodec::Decode(const CodedFrame &codedFrame)
{
	AVPacket *packet = codedFrame.InterpretAs<AVPacket>();

	VVCParser parser = {};
	CodecError ret = parser.Parse(packet->buf->data, packet->buf->size);
	if (ret != CodecError::Succeed)
	{
		return ret;
	}
}

CodecError VVCCodec::Parse(const CodedFrame &codedFrame)
{
	return CodecError::Succeed;
}

}
}

#endif
