#include "NVJpegCodec.h"

#if HAVE_NVJPEG
#include <nvjpeg.h>
#endif

namespace Immortal
{
namespace Vision
{

#if HAVE_NVJPEG

#define CHECK(ret)                     \
if ((ret) != cudaSuccess)              \
{									   \
	return CodecError::ExternalFailed; \
}

static int CudaDeviceMalloc(void **p, size_t s)
{
	return (int)cudaMalloc(p, s);
}

static int CudaDeviceFree(void *p)
{
	return (int)cudaFree(p);
}

static int CudaHostMalloc(void **p, size_t s, unsigned int f)
{
	return (int)cudaHostAlloc(p, s, f);
}

static int CudaHostFree(void *p)
{
	return (int)cudaFreeHost(p);
}

static inline Format GetNvJpegFormat(nvjpegOutputFormat_t outputFormat, nvjpegChromaSubsampling_t subsampling)
{
	if (outputFormat != NVJPEG_OUTPUT_YUV)
	{
		return Format::None;
	}

	switch (subsampling)
	{
	case NVJPEG_CSS_444:
		return Format::YUV444P;

	case NVJPEG_CSS_422:
		return Format::YUV422P;

	case NVJPEG_CSS_420:
		return Format::YUV420P;

	case NVJPEG_CSS_440:
	case NVJPEG_CSS_411:
	case NVJPEG_CSS_410:
	case NVJPEG_CSS_GRAY:
	case NVJPEG_CSS_410V:
	case NVJPEG_CSS_UNKNOWN:
	default:
		LOG::ERR("Incompatible nvjpeg format({}) - subsampling({})", (int)outputFormat, (int)subsampling);
		return Format::None;
	}
}

NVJpegCodec::NVJpegCodec() :
    Super{}
{

}

NVJpegCodec::~NVJpegCodec()
{

}

CodecError NVJpegCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.GetBuffer();

    return Decode(buffer.data(), buffer.size());
}

CodecError NVJpegCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
    return CodecError::NotImplement;
}
#endif

CodecError NVJpegCodec::Decode(const uint8_t *data, size_t size)
{
#if HAVE_NVJPEG
	nvjpegDevAllocator_t deviceAllocator = {
        .dev_malloc = &CudaDeviceMalloc,
        .dev_free   = &CudaDeviceFree
    };

	nvjpegPinnedAllocator_t pinnedAllocator = {
        .pinned_malloc = &CudaHostMalloc,
        .pinned_free   = &CudaHostFree
    };

    nvjpegHandle_t handle = {};
    int flags = 0;
	int batchSize = 1;

    nvjpegStatus_t status = NVJPEG_STATUS_SUCCESS;

    status = nvjpegCreateEx(NVJPEG_BACKEND_DEFAULT, &deviceAllocator, &pinnedAllocator, flags, &handle);
	if (status != NVJPEG_STATUS_SUCCESS)
    {
		return CodecError::ExternalFailed;
    }

	nvjpegOutputFormat_t outputFormat = NVJPEG_OUTPUT_YUV;

    int components = 0;
	int widths[NVJPEG_MAX_COMPONENT]  = {};
	int heights[NVJPEG_MAX_COMPONENT] = {};
	nvjpegChromaSubsampling_t subsampling = {};
	status = nvjpegGetImageInfo(handle, data, size, &components, &subsampling, widths, heights);
	if (status != NVJPEG_STATUS_SUCCESS)
	{
		return CodecError::ExternalFailed;
	}

	nvjpegImage_t image = {};
	for (int i = 0; i < components; i++)
	{
		image.pitch[i] = SLALIGN(widths[i], 64);
		auto size = 3 * image.pitch[i] * heights[i];
		cudaError_t error = cudaMalloc((void **)&image.channel[i], size);
		if (error != cudaSuccess)
		{
			for (int j = 0; j < i; j++)
			{
				error = cudaFree((void **) &image.channel[i]);
				if (error != cudaSuccess)
				{
					LOG::ERR("Failed to allocate and release a cuda memory!");
				}
			}
			return CodecError::ExternalFailed;
		}
	}

	cudaStream_t stream = {};
	cudaError_t error = cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking);
	if (error != cudaSuccess)
	{
		return CodecError::ExternalFailed;
	}

	CHECK(cudaStreamSynchronize(stream));

	cudaEvent_t startEvent = {};
	cudaEvent_t stopEvent  = {};
	CHECK(cudaEventCreate(&startEvent));
	CHECK(cudaEventCreate(&stopEvent));

	nvjpegJpegDecoder_t decoder = {};
	CHECK(nvjpegDecoderCreate(handle, NVJPEG_BACKEND_DEFAULT, &decoder));

	nvjpegJpegState_t decoupledState = {};
	CHECK(nvjpegDecoderStateCreate(handle, decoder, &decoupledState));

	nvjpegBufferPinned_t pinnedBuffers[2] = {};
	CHECK(nvjpegBufferPinnedCreate(handle, NULL, &pinnedBuffers[0]));
	CHECK(nvjpegBufferPinnedCreate(handle, NULL, &pinnedBuffers[1]));

	nvjpegBufferDevice_t deviceBuffer = {};
	CHECK(nvjpegBufferDeviceCreate(handle, NULL, &deviceBuffer));

	nvjpegJpegStream_t streams[2] = {};
	CHECK(nvjpegJpegStreamCreate(handle, &streams[0]));
	CHECK(nvjpegJpegStreamCreate(handle, &streams[1]));

	nvjpegDecodeParams_t decodeParams = {};
	CHECK(nvjpegDecodeParamsCreate(handle, &decodeParams));

	int bufferIndex = 0;
	CHECK(cudaEventRecord(startEvent, stream));
	CHECK(nvjpegStateAttachDeviceBuffer(decoupledState, deviceBuffer));
	CHECK(nvjpegDecodeParamsSetOutputFormat(decodeParams, outputFormat));
	CHECK(nvjpegJpegStreamParse(handle, data, size, 0, 0, streams[bufferIndex]));
	CHECK(nvjpegStateAttachPinnedBuffer(decoupledState, pinnedBuffers[bufferIndex]));
	CHECK(nvjpegDecodeJpegHost(handle, decoder, decoupledState, decodeParams, streams[bufferIndex]));
	CHECK(cudaStreamSynchronize(stream));
	CHECK(nvjpegDecodeJpegTransferToDevice(handle, decoder, decoupledState, streams[bufferIndex], stream));
	CHECK(nvjpegDecodeJpegDevice(handle, decoder, decoupledState, &image, stream));
	CHECK(cudaEventRecord(stopEvent, stream));

	CHECK(nvjpegDecodeParamsDestroy(decodeParams));
	CHECK(nvjpegJpegStreamDestroy(streams[0]));
	CHECK(nvjpegJpegStreamDestroy(streams[1]));
	CHECK(nvjpegBufferPinnedDestroy(pinnedBuffers[0]));
	CHECK(nvjpegBufferPinnedDestroy(pinnedBuffers[1]));
	CHECK(nvjpegBufferDeviceDestroy(deviceBuffer));
	CHECK(nvjpegJpegStateDestroy(decoupledState));
	CHECK(nvjpegDecoderDestroy(decoder));
	CHECK(nvjpegDestroy(handle));

	picture = Picture{ widths[0], heights[0], GetNvJpegFormat(outputFormat, subsampling) };

	for (int i = 0; i < NVJPEG_MAX_COMPONENT; i++)
	{
		picture.SetDataAt(i, image.channel[i]);
		picture.SetStride(i, image.pitch[i]);
	}
	picture.SetMemoryType(PictureMemoryType::Device);
	picture.SetRelease([=, this](void *data) {
		for (int i = 0; i < NVJPEG_MAX_COMPONENT; i++)
		{
			if (picture.GetData(i))
			{
				CHECK(cudaFree(picture.GetData(i)));
			}
		}
	});

    return CodecError::Success;
#else
	return CodecError::NotImplement;
#endif
}

}
}
