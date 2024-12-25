#include "Jxl.h"
#include <jxl/decode.h>
#include <jxl/encode.h>
#include <jxl/thread_parallel_runner.h>
#include <jxl/resizable_parallel_runner.h>
#include "Graphics/Types.h"

namespace Immortal
{
namespace Vision
{

#if HAVE_JXL
class JxlDecoderWrapper
{
public:
	JxlDecoderWrapper() :
	    handle{JxlDecoderCreate(nullptr)}
    {

    }

    ~JxlDecoderWrapper()
    {
		JxlDecoderDestroy(handle);
    }

    JxlDecoderStatus SubscribeEvents(int events)
    {
		return JxlDecoderSubscribeEvents(handle, events);
    }

    JxlDecoderStatus SetInput(const uint8_t *data, size_t size)
    {
		return JxlDecoderSetInput(handle, data, size);
    }

    void CloseInput()
    {
		JxlDecoderCloseInput(handle);
    }

	JxlDecoderStatus ProcessInput()
	{
		return JxlDecoderProcessInput(handle);
	}

	JxlDecoderStatus GetBasicInfo(JxlBasicInfo *pBasicInfo)
	{
		return JxlDecoderGetBasicInfo(handle, pBasicInfo);
	}

	JxlDecoderStatus SetImageOutBuffer(const JxlPixelFormat *format, void *buffer, size_t size)
	{
		return JxlDecoderSetImageOutBuffer(handle, format, buffer, size);
	}

	JxlDecoder *handle;
};

static inline auto GetTypeStr(JxlEncoderError error)
{
#ifdef _DEBUG
	switch (error)
	{
		case JXL_ENC_ERR_OK:
			return "OK";

		case JXL_ENC_ERR_GENERIC:
			return "Generic encoder error due to unspecified cause";

		case JXL_ENC_ERR_OOM:
			return "Out of memory";

		case JXL_ENC_ERR_JBRD:
			return "JPEG bitstream reconstruction data could not be represented(e.g.too much tail data)";

		case JXL_ENC_ERR_BAD_INPUT:
			return "Input is invalid (e.g. corrupt JPEG file or ICC profile)";

		case JXL_ENC_ERR_NOT_SUPPORTED:
			return "The encoder doesn't (yet) support this.   \n"
			       "Either no version of libjxl supports this,\n"
			       "and the API is used incorrectly, or the libjxl\n"
			       "version should have been checked before trying to do this.";

		case JXL_ENC_ERR_API_USAGE:
			return "The encoder API is used in an incorrect way\n"
			       "In this case, a debug build of libjxl should output a specific error\n"
			       "message. (if not, please open an issue about it)";

		default:
			return "";
	}
#else
	return (int) error;
#endif
}

class EncoderFrameSettings
{
public:
	EncoderFrameSettings() :
		handle{}
	{

	}

	EncoderFrameSettings(JxlEncoder *encoder, const EncoderFrameSettings &source = {}) :
	    handle{JxlEncoderFrameSettingsCreate(encoder, source.handle)}
	{

	}

	~EncoderFrameSettings()
	{

	}

	JxlEncoderStatus SetOption(JxlEncoderFrameSettingId option, float value)
	{
		JxlEncoderFrameSettingsSetFloatOption(handle, option, value);
	}

	JxlEncoderStatus SetOption(JxlEncoderFrameSettingId option, int64_t value)
	{
		JxlEncoderFrameSettingsSetOption(handle, option, value);
	}

	JxlEncoderStatus SetFrameLossless(int lossless)
	{
		return JxlEncoderSetFrameLossless(handle, lossless);
	}

	JxlEncoderFrameSettings *handle;
};

static void *libjxlMalloc(void *opaque, size_t size)
{
	return malloc(size);
}

static void libjxlFree(void *opaque, void *address)
{
	free(address);
}

class JxlEncoderWrapper
{
public:
	JxlEncoderWrapper() :
	    handle{},
	    runner{}
	{
		JxlMemoryManager manager = {
		    .opaque = nullptr,
			.alloc  = &libjxlMalloc,
			.free   = &libjxlFree
		};

		handle = JxlEncoderCreate(&manager);

		runner = JxlThreadParallelRunnerCreate(&manager, std::thread::hardware_concurrency());
		if (!runner)
		{
			LOG::ERR("Failed to create JxlThreadParallelRunner");
		}
	}

	~JxlEncoderWrapper()
	{
		JxlThreadParallelRunnerDestroy(runner);
		JxlEncoderDestroy(handle);
	}

	void InitBasicInfo(JxlBasicInfo *pBasicInfo)
	{
		JxlEncoderInitBasicInfo(pBasicInfo);
	}

	JxlEncoderStatus SetBasicInfo(const JxlBasicInfo *pBasicInfo)
	{
		return JxlEncoderSetBasicInfo(handle, pBasicInfo);
	}

	JxlEncoderStatus SetColorEncoding(const JxlColorEncoding *pColorEncoding)
	{
		return JxlEncoderSetColorEncoding(handle, pColorEncoding);
	}

	EncoderFrameSettings CreateEncoderFrameSettings()
	{
		return EncoderFrameSettings(handle);
	}

	JxlEncoderStatus AddImageFrame(const EncoderFrameSettings &frameSettings, const JxlPixelFormat *format, const void *data, size_t size)
	{
		return JxlEncoderAddImageFrame(frameSettings.handle, format, data, size);
	}

	JxlEncoderStatus ProcessOutput(uint8_t **nextOut, size_t *availOut)
	{
		return JxlEncoderProcessOutput(handle, nextOut, availOut);
	}

	auto GetError()
	{
		return GetTypeStr(JxlEncoderGetError(handle));
	}

	JxlEncoder *handle;

	void *runner;
};

JxlCodec::JxlCodec() :
    Super{}
{

}

JxlCodec::~JxlCodec()
{

}

CodecError JxlCodec::Decode(const CodedFrame &codedFrame)
{
    int width, height, depth;
    const auto &buffer = codedFrame.GetBuffer();

    JxlDecoderWrapper decoder{};
    JxlDecoderStatus status;
	status = decoder.SubscribeEvents(JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE);
	if (status != JxlDecoderStatus::JXL_DEC_SUCCESS)
    {
		LOG::ERR("[Jxl] Failed to subscribe events - {}", (int)status);
		return CodecError::ExternalFailed;
    }

    status = decoder.SetInput(buffer.data(), buffer.size());
	if (status != JxlDecoderStatus::JXL_DEC_SUCCESS)
	{
		LOG::ERR("[Jxl] Failed to set input - {}", (int)status);
		return CodecError::ExternalFailed;
	}
    decoder.CloseInput();

	JxlBasicInfo info;
	if (decoder.ProcessInput() != JXL_DEC_BASIC_INFO ||
	    decoder.GetBasicInfo(&info) != JXL_DEC_SUCCESS)
	{
		LOG::ERR("[Jxl] Failed to decode basic info");
		return CodecError::ExternalFailed;
	}
	
	JxlPixelFormat format = { 4, info.bits_per_sample == 16  ? JXL_TYPE_UINT16 : JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, TextureAlignment };
	picture = Picture{ info.xsize, info.ysize,  info.bits_per_sample == 16 ? Format::RGBA16 : Format::RGBA8, true };
	if (decoder.ProcessInput() != JXL_DEC_NEED_IMAGE_OUT_BUFFER ||
	    decoder.SetImageOutBuffer(&format, picture.GetData(), picture.GetStride(0) * picture.GetHeight()) != JXL_DEC_SUCCESS ||
	    decoder.ProcessInput() != JXL_DEC_FULL_IMAGE)
	{
		LOG::ERR("[Jxl] Failed to decode image");
		return CodecError::ExternalFailed;
	}

    return CodecError::Success;
}

static float Quality2Distance(float quality)
{
	if (quality >= 100.0)
	{
		return 0.0;
	}
	else if (quality >= 90.0)
	{
		return (100.0 - quality) * 0.10;
	}
	else if (quality >= 30.0)
	{
		return 0.1 + (100.0 - quality) * 0.09;
	}
	else if (quality > 0.0)
	{
		return 15.0 + (59.0 * quality - 4350.0) * quality / 9000.0;
	}
	else
	{
		return 15.0;
	}
}

CodecError JxlCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
	constexpr bool lossless = false;

	JxlEncoderWrapper encoder = JxlEncoderWrapper{};
	EncoderFrameSettings frameSettings = encoder.CreateEncoderFrameSettings();

	if (JxlEncoderSetParallelRunner(encoder.handle, JxlThreadParallelRunner, encoder.runner) != JXL_ENC_SUCCESS)
	{
		LOG::ERR("Failed to set JxlThreadParallelRunner- {}", encoder.GetError());
		return CodecError::ExternalFailed;
	}

	int effort = 7;
	if (JxlEncoderFrameSettingsSetOption(frameSettings.handle, JXL_ENC_FRAME_SETTING_EFFORT, effort) != JXL_ENC_SUCCESS)
	{
		LOG::ERR("Failed to set effort to: {}", effort);
		return CodecError::ExternalFailed;
	}

	int distance = -1.0f;
	int quality = 100.0f;
	if (distance < 0.0)
	{
		distance = Quality2Distance(quality);
		distance = 1.0;
	}

	if (distance > 0.0 && distance < 0.01)
	{
		distance = 0.01;
	}

	if (JxlEncoderSetFrameDistance(frameSettings.handle, distance) != JXL_ENC_SUCCESS)
	{
		LOG::ERR("Failed to set distance: {}", distance);
		return CodecError::ExternalFailed;
	}

	JxlBasicInfo info{};
	encoder.InitBasicInfo(&info);
	if (lossless)
	{
		info.uses_original_profile = true;
		frameSettings.SetFrameLossless(JXL_TRUE);
	}

	auto &format = picture.GetFormat();
	JxlPixelFormat pixelFormat = {
	    .num_channels = (uint32_t)format.GetComponent(),
		.endianness   = JXL_NATIVE_ENDIAN,
		.align        = picture.GetStride(0)
	};

	info.xsize = picture.GetWidth();
	info.ysize = picture.GetHeight();
	info.num_extra_channels = (pixelFormat.num_channels + 1) % 2;
	info.num_color_channels = pixelFormat.num_channels - info.num_extra_channels;

	if (format.IsType(Format::_16Bits) || format == Format::RGBA16)
	{
		info.bits_per_sample = 16;
	}
	else if (format.IsType(Format::_12Bits))
	{
		info.bits_per_sample = 12;
	}
	else if (format.IsType(Format::_10Bits))
	{
		info.bits_per_sample = 10;
	}
	else
	{
		info.bits_per_sample = 8;
	}
	info.alpha_bits = (info.num_extra_channels > 0) * info.bits_per_sample;

	info.exponent_bits_per_sample = 0;
	info.alpha_exponent_bits      = 0;
	pixelFormat.data_type = info.bits_per_sample <= 8 ? JXL_TYPE_UINT8 : JXL_TYPE_UINT16;
	
	info.uses_original_profile = distance == 0.0 || !false;
	info.orientation = picture.GetStride(0) >= 0 ? JXL_ORIENT_IDENTITY : JXL_ORIENT_FLIP_VERTICAL;

	if (encoder.SetBasicInfo(&info) != JXL_ENC_SUCCESS)
	{
		LOG::ERR("Failed to set basic info - {}", encoder.GetError());
		return CodecError::ExternalFailed;
	}

	JxlExtraChannelInfo extraChannelInfo{};
	JxlEncoderInitExtraChannelInfo(JXL_CHANNEL_ALPHA, &extraChannelInfo);
	if (JxlEncoderSetExtraChannelInfo(encoder.handle, 0, &extraChannelInfo) != JXL_ENC_SUCCESS)
	{
		LOG::ERR("Failed to set extra channel info - {}", encoder.GetError());
		return CodecError::ExternalFailed;
	}

	JxlColorEncoding colorEncoding = {
		.color_space       = info.num_color_channels == 1 ? JXL_COLOR_SPACE_GRAY : JXL_COLOR_SPACE_RGB,
	    .white_point       = JXL_WHITE_POINT_D65,
	    .primaries         = JXL_PRIMARIES_SRGB,
	    .transfer_function = JXL_TRANSFER_FUNCTION_709,
	    .rendering_intent  = JXL_RENDERING_INTENT_RELATIVE,
	};

	//JxlColorEncodingSetToSRGB(&colorEncoding, info.num_color_channels == 1);
	if (encoder.SetColorEncoding(&colorEncoding) != JXL_ENC_SUCCESS)
	{
		LOG::ERR("Failed to set color encoding - {}", encoder.GetError());
		return CodecError::ExternalFailed;
	}

	if (JxlEncoderGetRequiredCodestreamLevel(encoder.handle) > 5)
	{
		if (JxlEncoderSetCodestreamLevel(encoder.handle, 10) != JXL_ENC_SUCCESS)
		{
			LOG::WARN("Could not increase codestream level");
		}
	}

	auto size = picture.GetStride(0) * picture.GetHeight();
	if (encoder.AddImageFrame(frameSettings, &pixelFormat, picture.GetData(), size) != JXL_ENC_SUCCESS)
	{
		LOG::ERR("Failed to add image frame - {}", encoder.GetError());
		return CodecError::ExternalFailed;
	}

	JxlEncoderCloseInput(encoder.handle);

	std::vector<uint8_t> data;
	data.resize(size / 20);
	uint8_t *nextOut  = data.data();
	size_t   availOut = data.size();

	size_t usedSize = 0;
	while (true)
	{ 
		JxlEncoderStatus status = encoder.ProcessOutput(&nextOut, &availOut);
		if (status == JXL_ENC_ERROR)
		{
			LOG::ERR("Encoding error occurred - {}", encoder.GetError());
			return CodecError::ExternalFailed;
		}

		usedSize = data.size() - availOut;
		if (status == JXL_ENC_SUCCESS)
		{
			break;
		}
		if (status == JXL_ENC_NEED_MORE_OUTPUT)
		{ 
			data.resize(data.size() * 2);
			nextOut  = data.data() + usedSize;
			availOut = data.size() - usedSize;
		}
		else
		{
			LOG::ERR("Unknow libjxl status {}", (int) status);
			return CodecError::ExternalFailed;
		}
	}

	data.resize(usedSize);
	codedFrame = { std::move(data) };

    return CodecError::Success;
}

#endif

}
}
