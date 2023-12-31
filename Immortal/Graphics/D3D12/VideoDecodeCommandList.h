#pragma once

#include "Common.h"
#include <d3d12video.h>
#include "Device.h"

namespace Immortal
{

namespace D3D12
{

class VideoDecodeCommandList
{
public:
	using Primitive = ID3D12VideoDecodeCommandList;
	D3D12_OPERATOR_HANDLE()

	HRESULT Close()
	{
		return handle->Close();
	}

	HRESULT Reset(ID3D12CommandAllocator *pAllocator)
	{
		return handle->Reset(pAllocator);
	}

	void ClearState()
	{
		handle->ClearState();
	}

	void ResourceBarrier(UINT numBarriers, const D3D12_RESOURCE_BARRIER *pBarriers)
	{
		handle->ResourceBarrier(numBarriers, pBarriers);
	}

	void DiscardResource(ID3D12Resource *pResource, const D3D12_DISCARD_REGION *pRegion)
	{
		handle->DiscardResource(pResource, pRegion);
	}

	void BeginQuery(ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE type, UINT index)
	{
		handle->BeginQuery(pQueryHeap, type, index);
	}

	void EndQuery(ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE type, UINT index)
	{
		handle->EndQuery(pQueryHeap, type, index);
	}

	void ResolveQueryData(ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE type, UINT startIndex, UINT numQueries, ID3D12Resource *pDestinationBuffer, UINT64 alignedDestinationBufferOffset)
	{
		handle->ResolveQueryData(pQueryHeap, type, startIndex, numQueries, pDestinationBuffer, alignedDestinationBufferOffset);
	}

	void SetPredication(ID3D12Resource *pBuffer, UINT64 alignedBufferOffset, D3D12_PREDICATION_OP operation)
	{
		handle->SetPredication(pBuffer, alignedBufferOffset, operation);
	}

	void SetMarker(UINT metadata, const void *pData, UINT size)
	{
		handle->SetMarker(metadata, pData, size);
	}

	void BeginEvent(UINT metadata, const void *pData, UINT size)
	{
		handle->BeginEvent(metadata, pData, size);
	}

	void EndEvent()
	{
		handle->EndEvent();
	}

	void DecodeFrame(ID3D12VideoDecoder *pDecoder, const D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments, const D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments)
	{
		handle->DecodeFrame(pDecoder, pOutputArguments, pInputArguments);
	}

	void WriteBufferImmediate(UINT count, const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams, const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes)
	{
		handle->WriteBufferImmediate(count, pParams, pModes);
	}
};

}
}
