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
    if (ret < 0) {
        LOG::ERR("Error sending a packet for decoding\n");
        return CodecError::ExternalFailed;
    }

    if ((ret = avcodec_receive_frame(handle, frame)) == AVERROR(EAGAIN))
    {
        return CodecError::Again;
    }

    picture = Picture{ frame->width, frame->height, Format::RGBA8 };

    CVector<uint8_t> dst{};
    dst.x = picture.Data();

    CVector<uint8_t> src{};
    src.x = (uint8_t *)frame->data[0];
    src.y = (uint8_t *)frame->data[1];
    src.z = (uint8_t *)frame->data[2];
    src.linesize[0] = frame->linesize[0];
    src.linesize[1] = frame->linesize[1];

    auto coefficientType = handle->colorspace == AVCOL_SPC_BT709 ? ColorSpace::CoefficientType::REC709 : ColorSpace::CoefficientType::REC601;
    ColorSpace::YUV420PToRGBA8(dst, src, picture.desc.width, picture.desc.height, coefficientType);

    picture.pts = frame->pts / frame->pkt_duration;
    animator.Timestamps.Current = picture.pts;

    av_packet_unref(packet);
    av_frame_unref(frame);

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
