#pragma once

#include "Common.h"
#include "Math/Vector.h"
#include "Interface/IObject.h"
#include <d3d12video.h>

namespace Immortal
{

namespace D3D12
{

class Device;
class CommandAllocator;
class VideoDecoder;
class VideoDecoderHeap;
class VideoDecodeCommandList;
class VideoDevice : public IObject
{
public:
	using Primitive = ID3D12VideoDevice;
	D3D_OPERATOR_HANDLE()

public:
	VideoDevice();

	VideoDevice(Device *deivce);

	HRESULT CreateVideoDecoder(const D3D12_VIDEO_DECODER_DESC *pDesc, VideoDecoder *pVideoDecoder);

	HRESULT CreateVideoDecoderHeap(const D3D12_VIDEO_DECODER_HEAP_DESC *pDesc, VideoDecoderHeap *pVideoDecoder);

	HRESULT CreateVideoDecodeCommandList(CommandAllocator *pAllocator, VideoDecodeCommandList *pCommandList);

	HRESULT CheckDecoderSupport(const D3D12_VIDEO_DECODE_CONFIGURATION &configuration, DXGI_FORMAT format, UINT width, UINT height);

	HRESULT CheckFeatureSupport(D3D12_FEATURE_VIDEO featureVideo, void *pFeatureSupportData, UINT featureSupportDataSize)
	{
		return handle->CheckFeatureSupport(featureVideo, pFeatureSupportData, featureSupportDataSize);
	}

	HRESULT CreateVideoDecoder(const D3D12_VIDEO_DECODER_DESC *pDesc, ID3D12VideoDecoder **ppVideoDecoder)
	{
		return handle->CreateVideoDecoder(pDesc, IID_PPV_ARGS(ppVideoDecoder));
	}

	HRESULT CreateVideoDecoderHeap(const D3D12_VIDEO_DECODER_HEAP_DESC *pVideoDecoderHeapDesc, ID3D12VideoDecoderHeap **ppVideoDecoderHeap)
	{
		return handle->CreateVideoDecoderHeap(pVideoDecoderHeapDesc, IID_PPV_ARGS(ppVideoDecoderHeap));
	}

	HRESULT STDMETHODCALLTYPE CreateVideoProcessor(UINT nodeMask, const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC *pOutputStreamDesc, UINT numInputStreamDescs, const D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pInputStreamDescs, ID3D12VideoProcessor **ppVideoProcessor)
	{
		return handle->CreateVideoProcessor(nodeMask, pOutputStreamDesc, numInputStreamDescs, pInputStreamDescs, IID_PPV_ARGS(ppVideoProcessor));
	}

protected:
	Device *device;
};

}
}
