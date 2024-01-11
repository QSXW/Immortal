#include "FFCodec.h"
#include "Vision/Demux/FFDemuxer.h"
#include "Vision/Processing/ColorSpace.h"
#include "Render/Graphics.h"
#include "Audio/Device.h"
#include <list>

#if HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#ifdef _WIN32
#include <libavutil/hwcontext_d3d12va.h>
#include <d3d12.h>
#endif
}

#include "Shared/Async.h"

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
	case AVCOL_SPC_BT709:
    default:
		ret = ColorSpace::YUV_BT709;
        break;
    }

    return ret;
}

static inline Format CAST(AVPixelFormat v)
{
    switch (v)
    {
    case AV_PIX_FMT_YUV420P10:
        return Format::YUV420P10;

    case AV_PIX_FMT_YUV422P10:
        return Format::YUV422P10;

    case AV_PIX_FMT_YUV444P10:
        return Format::YUV444P10;

    case AV_PIX_FMT_NV12:
        return Format::NV12;

    case AV_PIX_FMT_P010LE:
        return Format::P010LE;

    case AV_PIX_FMT_Y210:
		return Format::Y210;
    default:
        return Format::YUV420P;
    }
}

FFCodec::FFCodec() :
    handle{},
    device{},
    type{ PictureMemoryType::System },
    startTimestamp{}
{
    frame = av_frame_alloc();
	ThrowIf(!frame, "FFCodec::Failed to allocated memory for frame!")

	memoryResource = new MemoryResource(sizeof(SharedPictureData));
}

FFCodec::~FFCodec()
{
    if (handle)
    {
        if (handle->extradata)
        {
		    av_freep(&handle->extradata);
        }
        av_frame_free(&frame);
        avcodec_free_context(&handle);
    }
    if (device)
    {
        av_buffer_unref(&device);
    }

    Flush();
	memoryResource.Reset();
}

#define RELEASE_PACKET av_packet_unref(packet); av_packet_free(&packet);
CodecError FFCodec::Decode(const CodedFrame &codedFrame)
{
    int ret = 0;
    auto packet = codedFrame.InterpretAs<AVPacket>();

    auto timeBase = packet->time_base;
    ret = avcodec_send_packet(handle, packet);
    if (ret < 0 && ret != AVERROR(EAGAIN))
    {
        if (ret != AVERROR(EOF))
        {
            RELEASE_PACKET
            return CodecError::ExternalFailed;
        }
    }
    RELEASE_PACKET

    ret = avcodec_receive_frame(handle, frame);
    if (ret < 0)
    {
        if (ret == AVERROR(EAGAIN))
        {
            return CodecError::Again;
        }
        return CodecError::ExternalFailed;
    }

    AVFrame *ref = NULL;
	if (device && type == PictureMemoryType::System)
    {
        ref = av_frame_alloc();
        if (!ref)
        {
            return CodecError::OutOfMemory;
        }
        if (av_hwframe_transfer_data(ref, frame, 0) < 0)
        {
            LOG::ERR("Failed to download frame to system memory!");
            av_frame_free(&ref);
            av_frame_unref(frame);

            return CodecError::ExternalFailed;
        }
    }
    else
    {
        ref = av_frame_clone(frame);
    }

    if (format == Format::None)
    {
		enum AVPixelFormat pixelFormat = handle->sw_pix_fmt;
		if (type == PictureMemoryType::Device && handle->pix_fmt != AV_PIX_FMT_YUV422P10)
		{
			switch (pixelFormat)
			{
				case AV_PIX_FMT_YUV444P10:
				case AV_PIX_FMT_YUV422P10:
				case AV_PIX_FMT_YUV420P10:
					pixelFormat = AV_PIX_FMT_P010;
					break;

                case AV_PIX_FMT_YUV444P:
                case AV_PIX_FMT_YUV422P:
				case AV_PIX_FMT_YUV420P:
				default:
					pixelFormat = AV_PIX_FMT_NV12;
			}
		}
		format = CAST(pixelFormat);
    }

    if (handle->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        picture = Picture{ref->width, ref->height, format };
		picture.SetStride(0, ref->linesize[0]);
		picture.SetStride(1, ref->linesize[1]);
		picture.SetStride(2, ref->linesize[2]);

#ifdef _WIN32
        if (handle->pix_fmt == AV_PIX_FMT_D3D12 && Graphics::GetDevice()->GetBackendAPI() == BackendAPI::D3D12)
        {
            auto &[texture, syncCtx] = *(AVD3D12VAFrame *)frame->data[0];
			picture.SetMemoryType(PictureMemoryType::Device);
            picture[0] = (uint8_t *)texture;
            picture[1] = (uint8_t *)syncCtx.fence;
            picture[2] = (uint8_t *)syncCtx.fence_value;
        }
#endif
        else
        {
			picture.SetMemoryType(PictureMemoryType::System);

			picture[0] = (uint8_t *) ref->data[0];
			picture[1] = (uint8_t *) ref->data[1];
			picture[2] = (uint8_t *) ref->data[2];
        }

        picture.SetRelease([ref] (void *) {
            Async::Execute([ref] {
                av_frame_unref(ref);
                av_frame_free((AVFrame **)&ref);
                });
            });
    }
    else if (handle->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        AVRational tb{ 1, frame->sample_rate };
        if (frame->pts != AV_NOPTS_VALUE)
        {
            frame->pts = av_rescale_q(frame->pts - startTimestamp, timeBase, tb);
        }

        SwrContext *swrContext = swr_alloc_set_opts(
            nullptr,
            AV_CH_LAYOUT_STEREO,
            AV_SAMPLE_FMT_FLT,
            AudioDevice::GetSampleRate(),
            !handle->channel_layout ? AV_CH_LAYOUT_STEREO : handle->channel_layout,
            handle->sample_fmt,
            handle->sample_rate,
            0,
            nullptr
        );

        swr_init(swrContext);

        int samples = av_rescale_rnd(
            swr_get_delay(swrContext, handle->sample_rate) + frame->nb_samples,
            AudioDevice::GetSampleRate(),
            handle->sample_rate,
            AV_ROUND_UP
        );

        Format format = Format::VECTOR2;
        picture = Picture{ uint32_t(samples), 1,  format, true };
		picture.SetTimestamp(frame->pts / frame->nb_samples);

        int outSamples = swr_convert(swrContext, &picture.GetData(), samples, (const uint8_t **) &frame->data[0], frame->nb_samples);
        if (outSamples < 0)
        {
            LOG::ERR("Failed to rescale audio frame format!");
        }

        uint8_t *ptr = picture.GetData() + outSamples * 2 * sizeof(float);
        if (swr_get_out_samples(swrContext, 0) > 0)
        {
            outSamples = swr_convert(swrContext, &ptr, samples - outSamples, nullptr, 0);
        }

        swr_free(&swrContext);
    }

    picture.SetTimestamp(NAN);
    if (frame->pts != AV_NOPTS_VALUE)
    {
        picture.SetTimestamp((frame->best_effort_timestamp - startTimestamp) * av_q2d(timeBase) * animator.FramesPerSecond);
    }

    av_frame_unref(frame);

    return CodecError::Succeed;
}

Picture FFCodec::GetPicture() const
{
    return picture;
}

void FFCodec::Flush()
{
    picture = Picture{};
}

AVHWDeviceType GetDeviceType(const std::string &name)
{
    if (name.find("qsv") != std::string::npos)
    {
		return AV_HWDEVICE_TYPE_QSV;
    }
    if (name.find("cuvid") != std::string::npos || name.find("nvdec") != std::string::npos)
    {
		return AV_HWDEVICE_TYPE_CUDA;
    }

    return AV_HWDEVICE_TYPE_NONE;
}

static std::list<const char *> QueryDecoderPriorities(const AVCodecID id)
{
    switch (id)
    {
    case AV_CODEC_ID_H264:
        return {
            "h264_cuvid",
            "h264_qsv",
        };

    case AV_CODEC_ID_HEVC:
        return {
			"hevc_qsv",
            "hevc_cuvid",
        };

    case AV_CODEC_ID_AV1:
        return {
			"av1_qsv",
            "libdav1d",
            "av1_nvdec",
        };

    case AV_CODEC_ID_MJPEG:
        return {
            "mjpeg_nvdec",
            "mjpeg_qsv",
        };

    case AV_CODEC_ID_MPEG4:
        return {
            "mpeg4_nvdec"
        };

    case AV_CODEC_ID_MPEG1VIDEO:
        return {
            "mpeg1_nvdec"
        };

    case AV_CODEC_ID_MPEG2VIDEO:
        return {
            "mpeg2_nvdec",
            "mpeg2_qsv",
        };

    case AV_CODEC_ID_VC1:
        return {
            "vc1_nvdec"
        };

    case AV_CODEC_ID_WMV3:
        return {
            "wmv3_nvdec"
        };

    case AV_CODEC_ID_VP8:
        return {
            "vp8_nvdec"
        };

    case AV_CODEC_ID_VP9:
        return {
            "vp9_nvdec",
            "vp9_qsv",
        };

    default:
        return {};
    }
}

static AVHWDeviceType QueryDecoderHWAccelType()
{
    AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;

    static std::list<const char *> priorities = {
	    "d3d12va",
    };

    for (auto &p : priorities)
    {
        type = av_hwdevice_find_type_by_name(p);
        if (type != AV_HWDEVICE_TYPE_NONE)
        {
            break;
        }
    }

    return type;
}

CodecError FFCodec::SetCodecContext(Anonymous anonymous)
{
    auto params = (FFDemuxer::Params *)(anonymous);
    const AVStream *stream = params->stream;
    startTimestamp = stream->start_time;

    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
    {
        return CodecError::NotImplement;
    }

 #ifdef _WIN32
    auto hwaccelType = QueryDecoderHWAccelType();
    for (int i = 0; ; i++)
    {
        const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
        if (!config)
        {
            break;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == hwaccelType)
        {

			if (hwaccelType == AV_HWDEVICE_TYPE_D3D12VA && Graphics::GetDevice()->GetBackendAPI() == BackendAPI::D3D12)
            {
			    device = av_hwdevice_ctx_alloc(hwaccelType);
                if (!device)
                {
				    return CodecError::OutOfMemory;
                }

			    AVHWDeviceContext *deviceContext  = (AVHWDeviceContext *)device->data;
			    AVD3D12VADeviceContext *d3d12vaContext = (AVD3D12VADeviceContext *)deviceContext->hwctx;

			    d3d12vaContext->device = (ID3D12Device *)Graphics::GetDevice()->GetBackendHandle();
			    d3d12vaContext->device->AddRef();
                av_hwdevice_ctx_init(device);

			    type = PictureMemoryType::Device;
            }
            else if (av_hwdevice_ctx_create(&device, hwaccelType, NULL, NULL, 0) < 0)
            {
                LOG::ERR("Failed to create specified HW device.");
                return CodecError::NotImplement;
            }
            break;
        }
    }
#endif

    if (!device)
    {
        auto priorities = QueryDecoderPriorities(stream->codecpar->codec_id);
        for (auto p : priorities)
        {
            const AVCodec *externalCodec = avcodec_find_decoder_by_name(p);
            if (externalCodec)
            {
				auto type = GetDeviceType(p);
                if (type != AV_HWDEVICE_TYPE_NONE)
                {
					if (av_hwdevice_ctx_create(&device, type, "auto", NULL, 0) < 0)
					{
						LOG::ERR("Cannot open the hardware device\n");
						continue;
					}
                }
                codec = externalCodec;
                break;
            }
        }
    }

    handle = avcodec_alloc_context3(codec);
    if (!handle)
    {
		LOG::ERR("Failed to allocate decode context!");
		return CodecError::OutOfMemory;
    }

    if (device)
    {
		handle->hw_device_ctx = av_buffer_ref(device);
    }
    if (stream->codecpar->extradata_size)
	{
		handle->extradata = (uint8_t *)av_mallocz(stream->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
		if (!handle->extradata)
		{
			LOG::ERR("Failed to allocate extra data!");
			return CodecError::OutOfMemory;
		}
		memcpy(handle->extradata, stream->codecpar->extradata, stream->codecpar->extradata_size);
		handle->extradata_size = stream->codecpar->extradata_size;
	}

    int ret = avcodec_parameters_to_context(handle, stream->codecpar);
    if (ret < 0)
    {
        LOG::ERR("FFCodec::Failed to fill AVCodecContext parameters");
    }

    handle->pkt_timebase = stream->time_base;

    AVDictionary **opts = (AVDictionary**)av_calloc(1, sizeof(*opts));
    //av_dict_set(opts, "threads", "16", 0);
    if (avcodec_open2(handle, codec, opts) < 0)
    {
        LOG::ERR("FFCodec::Failed to open AVCodecContext");
    }

    av_dict_free(opts);

    return CodecError::Succeed;
}

}
}
#endif
