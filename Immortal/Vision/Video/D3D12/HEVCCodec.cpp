#include "HEVCCodec.h"
#include "Render/Graphics.h"

#if HAVE_FFMPEG
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
#endif

namespace Immortal
{
namespace Vision
{
namespace D3D12
{

#if HAVE_FFMPEG
HEVCCodec::HEVCCodec()
{

}

HEVCCodec::~HEVCCodec()
{

}

struct SideData
{
	uint8_t *data;
	size_t size;
};

int find_next_start_code(const uint8_t *buf, const uint8_t *next_avc)
{
	int i = 0;

	if (buf + 3 >= next_avc)
		return next_avc - buf;

	while (buf + i + 3 < next_avc)
	{
		if (buf[i] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1)
			break;
		i++;
	}
	return i + 3;
}

CodecError HEVCCodec::Decode(const CodedFrame &codedFrame)
{
	SideData sizeData;

	auto packet = codedFrame.InterpretAs<AVPacket>();

	if (!packet->size)
	{
		return CodecError::Success;
	}

	sizeData.data = av_packet_get_side_data(packet, AV_PKT_DATA_NEW_EXTRADATA, &sizeData.size);
	if (sizeData.data && sizeData.size > 0)
	{
		return CodecError::Success;
	}

	sizeData.data = av_packet_get_side_data(packet, AV_PKT_DATA_DOVI_CONF, &sizeData.size);
	if (sizeData.data && sizeData.size > 0)
	{
		return CodecError::Success;
	}
	int i;
	int eos_at_start = 1;

	BitTracker bitTracker(packet->data, packet->size);
	CodecError ret = Parse(packet->buf->data, packet->buf->size);

	return ret;
}
#endif

}
}
}
