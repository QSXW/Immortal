#pragma once

#include "Common.h"
#include <d3d12video.h>

namespace Immortal
{

namespace D3D12
{

class VideoDecoderHeap
{
public:
	using Primitive = ID3D12VideoDecoderHeap;
	D3D_OPERATOR_HANDLE()

	D3D12_VIDEO_DECODER_HEAP_DESC GetDesc()
	{
		return handle->GetDesc();
	}
};

}
}
