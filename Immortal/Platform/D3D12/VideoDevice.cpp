#include "VideoDevice.h"

#include "Common.h"
#include <d3d12video.h>
#include "Device.h"
#include "CommandAllocator.h"
#include "VideoDecoder.h"
#include "VideoDecoderHeap.h"
#include "VideoDecodeCommandList.h"

namespace Immortal
{

namespace D3D12
{

VideoDevice::VideoDevice() :
    device{}
{

}

VideoDevice::VideoDevice(Device *device) :
    device{ device }
{
	device->QueryInterface(handle.GetAddressOf());
}

HRESULT VideoDevice::CreateVideoDecoder(const D3D12_VIDEO_DECODER_DESC *pDesc, VideoDecoder *pVideoDecoder)
{
	return CreateVideoDecoder(pDesc, pVideoDecoder->AddressOf());
}

HRESULT VideoDevice::CreateVideoDecoderHeap(const D3D12_VIDEO_DECODER_HEAP_DESC *pDesc, VideoDecoderHeap *pVideoDecoderHeap)
{
	return CreateVideoDecoderHeap(pDesc, pVideoDecoderHeap->AddressOf());
}

HRESULT VideoDevice::CreateVideoDecodeCommandList(CommandAllocator *pAllocator, VideoDecodeCommandList *pCommandList)
{
	return device->CreateCommandList(
		D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
	    *pAllocator,
	    nullptr,
	    pCommandList->AddressOf());
}

HRESULT VideoDevice::CheckDecoderSupport(const D3D12_VIDEO_DECODE_CONFIGURATION &configuration, DXGI_FORMAT format, UINT width, UINT height)
{
	HRESULT ret;
	
	D3D12_VIDEO_DECODE_TIER tier = D3D12_VIDEO_DECODE_TIER_NOT_SUPPORTED;
	D3D12_VIDEO_DECODE_CONFIGURATION_FLAGS configFlags =D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_NONE;

	D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT feature = {
	    .NodeIndex     = 0,
	    .Configuration = configuration,
	    .Width         = width,
	    .Height        = height,
	    .DecodeFormat  = format,
	    .FrameRate     = { 0, 1 },
	    .BitRate       = 0,
	};

	ret = CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_SUPPORT, &feature, sizeof(feature));
	if (FAILED(ret))
	{
		return ret;
	}

	if (!(feature.SupportFlags & D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED))
	{
		char uuid[64] = {};
		Guid2String(uuid, configuration.DecodeProfile);

		LOG::INFO("D3D12 video device doesn't support:\n",
		          ".Configuration = (.guid = {}, .BitstreamEncryption = {}, .InterlaceType = {})\n"
		          ".Width         = {}\n"
		          ".Height        = {}\n"
		          ".DecodeFormat  = {}\n"
		          ".FrameRate     = {}/{}\n"
		          ".BitRate       = {}",
		          uuid, configuration.BitstreamEncryption, configuration.InterlaceType,
		          width,
		          height,
		          TypeString(format),
		          feature.FrameRate.Numerator, feature.FrameRate.Denominator,
		          feature.BitRate
		);

		return S_FALSE;
	}
	
	return ret;
}

}
}
