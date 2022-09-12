#include "FFCodec.h"
#include "Vision/Processing/ColorSpace.h"

#if HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
}

namespace Immortal
{
namespace Vision
{

static inline ColorSpace ColorSpaceConverter(AVColorSpace v)
{
    ColorSpace ret = ColorSpace::YUV;

    switch (v)
    {
    case AVCOL_SPC_RGB:
        break;
    case AVCOL_SPC_BT709:
        ret = ColorSpace::YUV_BT709;
        break;
    case AVCOL_SPC_UNSPECIFIED:
        break;
    case AVCOL_SPC_RESERVED:
        break;
    case AVCOL_SPC_FCC:
        break;
    case AVCOL_SPC_BT470BG:
        break;
    case AVCOL_SPC_SMPTE170M:
        break;
    case AVCOL_SPC_SMPTE240M:
        break;
    case AVCOL_SPC_YCGCO:
        break;
    case AVCOL_SPC_BT2020_NCL:
        break;
    case AVCOL_SPC_BT2020_CL:
        break;
    case AVCOL_SPC_SMPTE2085:
        break;
    case AVCOL_SPC_CHROMA_DERIVED_NCL:
        break;
    case AVCOL_SPC_CHROMA_DERIVED_CL:
        break;
    case AVCOL_SPC_ICTCP:
        break;
    case AVCOL_SPC_NB:
        break;
    default:
        break;
    }

    return ret;
}

static inline Format FormatConverter(AVPixelFormat v)
{
    switch (v)
    {
    case AV_PIX_FMT_YUV420P10LE:
		return Format::YUV420P10;

    case AV_PIX_FMT_YUV422P10LE:
		return Format::YUV422P10;

    case AV_PIX_FMT_YUV444P10LE:
		return Format::YUV444P10;

	case AV_PIX_FMT_NV12:
		return Format::NV12;

    case AV_PIX_FMT_P010LE:
		return Format::P010LE;

    default:
        return Format::YUV420P;
    }
}

FFCodec::FFCodec()
{
    handle = avcodec_alloc_context3(nullptr);
    ThrowIf(!handle, "FFCodec::Failed to allocated memory!")

    frame = av_frame_alloc();
    ThrowIf(!frame, "FFCodec::Failed to allocated memory for frame!")
}

FFCodec::~FFCodec()
{
    if (handle)
    {
        av_frame_free(&frame);
        avcodec_free_context(&handle);
    }
}

CodecError FFCodec::Decode(const CodedFrame &codedFrame)
{
    int ret = 0; 
    auto packet = codedFrame.DeRef<AVPacket>();

    ret = avcodec_send_packet(handle, packet);
    if (ret < 0 && ret != AVERROR(EAGAIN))
    {
        if (ret != AVERROR(EOF))
        {
			av_packet_unref(packet);
			return CodecError::ExternalFailed;
        }
    }

	ret = avcodec_receive_frame(handle, frame);
    if (ret < 0)
	{
        if (ret == AVERROR(EAGAIN))
        {
			return CodecError::Again;
        }
		return CodecError::ExternalFailed;
	}

    picture = Picture{ frame->width, frame->height, FormatConverter(handle->pix_fmt), false };

    picture[0] = (uint8_t *)frame->data[0];
    picture[1] = (uint8_t *)frame->data[1];
    picture[2] = (uint8_t *)frame->data[2];

    memcpy(picture.shared->linesize, frame->linesize, sizeof(frame->linesize));

    AVFrame *ref = av_frame_clone(frame);
    picture.Connect([ref] {
        av_frame_unref(ref);
        av_frame_free((AVFrame**)&ref);
    });
    picture.SetProperty(ColorSpaceConverter(handle->colorspace));

    // picture.pts = frame->pts * av_q2d(packet->time_base) * animator.FramesPerSecond;
    animator.Timestamps.Current = picture.pts;
    av_packet_unref(packet);

    return CodecError::Succeed;
}

uint8_t * FFCodec::Data() const
{
    return nullptr;
}

Picture FFCodec::GetPicture() const
{
    return picture;
}

void FFCodec::SetCodecContext(const AVCodec *codec, const AVCodecParameters *parameters)
{
    int ret = avcodec_parameters_to_context(handle, parameters);
    if (ret < 0)
    {
        LOG::ERR("FFCodec::Failed to fill AVCodecContext parameters");
    }

    AVDictionary **opts = (AVDictionary**)av_calloc(1, sizeof(*opts));
    av_dict_set(opts, "threads", "auto", 0);
    if (avcodec_open2(handle, codec, opts) < 0)
    {
        LOG::ERR("FFCodec::Failed to open AVCodecContext");
    }

    av_dict_free(opts);
}

}
}
#endif
