#include "CommandBuffer.h"
#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "DescriptorSet.h"
#include "Sampler.h"

namespace Immortal
{
namespace D3D12
{

void SetGraphicsRootDescriptorTable(CommandList *commandList, uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
	commandList->SetGraphicsRootDescriptorTable(index, baseDescriptor);
}

void SetComputeRootDescriptorTable(CommandList *commandList, uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
	commandList->SetComputeRootDescriptorTable(index, baseDescriptor);
}

CommandBuffer::CommandBuffer(Device *device, D3D12_COMMAND_LIST_TYPE type) :
    NonDispatchableHandle{device},
    allocatorPool{ device, type },
    allocator{},
    activeBarrier{},
    renderTarget{}
{
	allocator = allocatorPool.RequestAllocator(0);
	commandList = { device, CommandList::Type(type), allocator };
	commandList.Close();

#ifdef _DEBUG
	commandList.SetName(L"CommandBuffer::CommandList");
#endif
}

CommandBuffer::~CommandBuffer()
{
	descriptorSets.clear();
}

void CommandBuffer::SetGraphicsPipeline(GraphicsPipeline *graphicsPipeline)
{
	commandList.SetPipelineState(*graphicsPipeline);
	commandList.SetGraphicsRootSignature(graphicsPipeline->GetRootSignature());
	commandList.SetPrimitiveTopology(graphicsPipeline->GetPrimitiveTopology());
}

void CommandBuffer::SetComputePipeline(ComputePipeline *computePipeline)
{
	commandList.SetPipelineState(*computePipeline);
	commandList.SetComputeRootSignature(computePipeline->GetRootSignature());
}

bool CommandBuffer::IsActive()
{
	return false;
}

void CommandBuffer::Reset()
{
	if (!commandList.IsState(CommandList::State::Reset))
	{
		allocator->Reset();
		commandList.SetState(CommandList::State::Reset);
		commandList.Reset(allocator);
	}
}

void CommandBuffer::Begin()
{
	Reset();
	descriptorSets.clear();
}

void CommandBuffer::End()
{
	commandList.Close();
}

void CommandBuffer::Close()
{
	commandList.Close();
}

void CommandBuffer::BeginEvent(const char *pData, size_t size)
{

}

void CommandBuffer::EndEvent()
{

}

void CommandBuffer::SetPipeline(SuperPipeline *_pipeline)
{
	pipeline = InterpretAs<Pipeline>(_pipeline);

	if (pipeline->GetType() == Pipeline::Type::Graphics)
	{
		SetRootDescriptorTable = SetGraphicsRootDescriptorTable;
		SetGraphicsPipeline(InterpretAs<GraphicsPipeline>(_pipeline));
	}
	else if (pipeline->GetType() == Pipeline::Type::Compute)
	{
		SetRootDescriptorTable = SetComputeRootDescriptorTable;
		SetComputePipeline(InterpretAs<ComputePipeline>(_pipeline));
	}
}

void CommandBuffer::SetDescriptorSet(SuperDescriptorSet *_descriptorSet)
{
	DescriptorSet *descriptorSet = InterpretAs<DescriptorSet>(_descriptorSet);
	DescriptorHeap *shaderResourceDescriptorHeap = descriptorSet->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DescriptorHeap *samplerDescriptorHeap = descriptorSet->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	ID3D12DescriptorHeap *ppDescriptorHeap[2] = { *shaderResourceDescriptorHeap };
	if (samplerDescriptorHeap)
	{
		ppDescriptorHeap[1] = *samplerDescriptorHeap;
	}
	commandList.SetDescriptorHeaps(ppDescriptorHeap, samplerDescriptorHeap ? SL_ARRAY_LENGTH(ppDescriptorHeap) : 1);

	auto &descriptorTables = pipeline->GetDescriptorTables();
	for (uint32_t i = 0; i < descriptorTables.size(); i++)
	{
		auto &descriptorTable = descriptorTables[i];
		ShaderVisibleDescriptor descriptor = descriptorSet->GetDescriptors(descriptorTable.HeapType);
		DescriptorHeap *descriptorHeap = descriptorSet->GetDescriptorHeap(descriptorTable.HeapType);
		SetRootDescriptorTable(&commandList, descriptorTables[i].RootParameterIndex, descriptor[descriptorTables[i].Offset]);
	}
}

void CommandBuffer::SetVertexBuffers(uint32_t firstSlot, uint32_t bufferCount, SuperBuffer **ppBuffer, uint32_t strideInBytes)
{
	std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
	views.resize(bufferCount);

	for (size_t i = 0; i < bufferCount; i++)
	{
		Buffer *buffer = InterpretAs<Buffer>(ppBuffer[i]);
		views[i] = {
		    .BufferLocation = buffer->GetGPUVirtualAddress(),
			.SizeInBytes    = UINT(buffer->GetSize()),
			.StrideInBytes  = strideInBytes,
			};
	}

	commandList.SetVertexBuffers(views.data(), bufferCount, firstSlot);
}

void CommandBuffer::SetIndexBuffer(SuperBuffer *_0, Format format)
{
	Buffer *buffer = InterpretAs<Buffer>(_0);

	D3D12_INDEX_BUFFER_VIEW view = {
	    .BufferLocation = buffer->GetGPUVirtualAddress(),
		.SizeInBytes    = uint32_t(buffer->GetSize()),
		.Format         = format
	};

	commandList.SetIndexBuffer(&view);
}

void CommandBuffer::SetScissors(uint32_t count, const Rect2D *pScissor)
{
	D3D12_RECT *pRect = (D3D12_RECT *)pScissor;
	commandList.RSSetScissorRects(pRect, count);
}

void CommandBuffer::SetBlendFactor(const float factor[4])
{
	commandList.OMSetBlendFactor(factor);
}

void CommandBuffer::PushConstants(ShaderStage stage, const void *pData, uint32_t size, uint32_t offset)
{
	switch (stage)
	{
		case ShaderStage::Compute:
			commandList.PushComputeConstant(size, pData, offset);
			break;

		default:
			commandList.PushGraphicsConstant(size, pData, offset);
			break;
	}
}

void CommandBuffer::BeginRenderTarget(SuperRenderTarget *_renderTarget, const float *pClearColor)
{
	renderTarget = InterpretAs<RenderTarget>(_renderTarget);

	auto width  = renderTarget->GetWidth();
	auto height = renderTarget->GetHeight();

	Viewport viewport{ 0, 0, width, height };
	commandList.RSSetViewports(&viewport);

	D3D12_RECT scissorRect{ 0, 0, (LONG)width, (LONG)height };
	commandList.RSSetScissorRects(&scissorRect);

	auto &colorBuffers = renderTarget->GetColorBuffers();
	Texture *depthBuffer  = renderTarget->GetDepthBuffer();

	for (auto &colorBuffer : colorBuffers)
	{
		barriers[activeBarrier++].Transition(
		    *colorBuffer,
		    D3D12_RESOURCE_STATE_COMMON,
		    D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	if (depthBuffer)
	{
		barriers[activeBarrier++].Transition(
		    *depthBuffer,
		    D3D12_RESOURCE_STATE_COMMON,
		    D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	commandList.ResourceBarrier(barriers.data(), activeBarrier);

	const auto &rtvDescriptor = renderTarget->GetDescriptor();
	const auto &dsvDescriptor = renderTarget->GetDepthBufferDescriptor();
	commandList.ClearRenderTargetView(rtvDescriptor[0], pClearColor);

	if (dsvDescriptor.ptr)
	{
		commandList.ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);
		commandList.SetRenderTargets(rtvDescriptor, uint32_t(colorBuffers.size()), false, &dsvDescriptor);
	}
	else
	{
		commandList.SetRenderTargets(rtvDescriptor, uint32_t(colorBuffers.size()), false, nullptr);
	}
}

void CommandBuffer::EndRenderTarget()
{
	for (auto &barrier : barriers)
	{
		barrier.Swap();
	}
	commandList.ResourceBarrier(barriers.data(), activeBarrier);
	activeBarrier = 0;

	auto &textures = renderTarget->GetColorBuffers();
	for (auto &texture : textures)
	{
		if (texture->GetMipLevels() > 1)
		{
			GenerateMipMaps(texture, Filter::Linear);
		}
	}

	renderTarget = nullptr;
}

void CommandBuffer::GenerateMipMaps(SuperTexture *_texture, Filter filter)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	uint32_t mipLevels = texture->GetMipLevels();
	if (mipLevels <= 1)
	{
		return;
	}

	Sampler *sampler = device->GetSampler(filter);
	Pipeline *pipeline = device->GetPipeline("GenerateMipMaps");

	descriptorSets.reserve(descriptorSets.size() + mipLevels);
	Barrier<BarrierType::Transition> barrier{ *texture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS };
	commandList.ResourceBarrier(&barrier);
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		URef<DescriptorSet>	descriptorSet = new DescriptorSet{ device, pipeline };
		uint32_t width  = texture->GetWidth() >> i;
		uint32_t height = texture->GetHeight() >> i;
		descriptorSet->Set(0, sampler);
		descriptorSet->Set(0, texture->GetDescriptor(i - 1), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		descriptorSet->Set(1, texture->GetUAVDescriptor(i),  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		SetPipeline(pipeline);
		SetDescriptorSet(descriptorSet);
		descriptorSets.emplace_back(std::move(descriptorSet));

		float ratio[] = {
		    1.0f / width,
		    1.0f / height,
		};

		PushConstants(ShaderStage::Compute, ratio, sizeof(ratio), 0);
		Dispatch(std::max(SLALIGN(width / 8, 8), 1u), std::max(SLALIGN(height / 8, 8), 1u), 1);

		Barrier<BarrierType::UAV> barrier{ *texture };
		commandList.ResourceBarrier(&barrier);
	}
	barrier.Swap();
	commandList.ResourceBarrier(&barrier);
}

void CommandBuffer::CopyBufferToImage(SuperTexture *_texture, uint32_t subresource, SuperBuffer *_buffer, size_t bufferRowLength, uint32_t offset)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	Buffer *buffer   = InterpretAs<Buffer>(_buffer);

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {
		.pResource = *buffer,
		.Type      = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
		.PlacedFootprint = {
	        .Offset = offset,
			.Footprint = {
	            .Format   = texture->GetFormat(),
	            .Width    = texture->GetWidth(),
	            .Height   = texture->GetHeight(),
	            .Depth    = 1,
	            .RowPitch = (UINT)bufferRowLength
			}
		}
	};

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {
		.pResource        = *texture,
		.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
	    .SubresourceIndex = subresource
	};

	auto state = texture->GetState();

	Barrier<BarrierType::Transition> barrier{
        *texture,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
    };

	if (!(state & D3D12_RESOURCE_STATE_COPY_DEST))
	{
		commandList.ResourceBarrier(&barrier, 1);
	}

	commandList.CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	barrier.Swap();
	commandList.ResourceBarrier(&barrier, 1);
	texture->SetState(D3D12_RESOURCE_STATE_COMMON);
}

void CommandBuffer::CopyImageToBuffer(SuperBuffer *_buffer, SuperTexture *_texture, uint32_t subresource, size_t bufferRowLength)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	Buffer *buffer   = InterpretAs<Buffer>(_buffer);

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {
		.pResource = *buffer,
		.Type      = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
		.PlacedFootprint = {
	        .Offset = 0,
			.Footprint = {
	            .Format   = texture->GetFormat(),
	            .Width    = texture->GetWidth(),
	            .Height   = texture->GetHeight(),
	            .Depth    = 1,
	            .RowPitch = (UINT)bufferRowLength
			}
		}
	};

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {
		.pResource        = *texture,
		.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
	    .SubresourceIndex = subresource
	};

	auto state = texture->GetState();
	Barrier<BarrierType::Transition> barrier{
        *texture,
        D3D12_RESOURCE_STATE_COMMON,
	    D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
    };

	if (!(state & D3D12_RESOURCE_STATE_COPY_SOURCE))
	{
		commandList.ResourceBarrier(&barrier, 1);
	}

	commandList.CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	barrier.Swap();
	commandList.ResourceBarrier(&barrier, 1);
	texture->SetState(D3D12_RESOURCE_STATE_COMMON);
}

void CommandBuffer::CopyPlatformSpecificSubresource(SuperTexture *dst, uint32_t dstSubresource, void *src, uint32_t srcSubresource)
{
	Texture *texture = InterpretAs<Texture>(dst);
	D3D12_TEXTURE_COPY_LOCATION srcLocation = {
		.pResource        = (ID3D12Resource *)src,
		.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
	    .SubresourceIndex = srcSubresource
	};

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {
	    .pResource        = *texture,
		.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
	    .SubresourceIndex = dstSubresource,
	};

	Barrier<Transition> barriers[] = {
		{ srcLocation.pResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE, srcSubresource  },
		{ dstLocation.pResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST,   dstSubresource  },
	};

	auto state = texture->GetState();
	if (!(state & D3D12_RESOURCE_STATE_COPY_DEST))
	{
		commandList.ResourceBarrier(barriers, SL_ARRAY_LENGTH(barriers));
	}
	else
	{
		commandList.ResourceBarrier(barriers, 1);
	}

	commandList.CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

	barriers[0].Swap();
	barriers[1].Swap();
	commandList.ResourceBarrier(barriers, SL_ARRAY_LENGTH(barriers));
	texture->SetState(D3D12_RESOURCE_STATE_COMMON);
}

void CommandBuffer::MemoryCopy(SuperBuffer *_buffer, uint32_t size, const void *data, uint32_t offset)
{
	SLASSERT(false && "Don't call this function for D3D12 backend!");
}

void CommandBuffer::MemoryCopy(SuperTexture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch)
{
	SLASSERT(false && "Don't call this function for Vulkan backend!");
}

void CommandBuffer::MemoryCopy(SuperBuffer *_dst, uint32_t dstOffset, SuperBuffer *_src, uint32_t srcOffset, size_t size)
{
	Buffer *dst = InterpretAs<Buffer>(_dst);
	Buffer *src = InterpretAs<Buffer>(_src);

	Barrier<Transition> barrier(*dst, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	commandList.ResourceBarrier(&barrier, 1);
	commandList.Handle()->CopyBufferRegion(*dst, dstOffset, *src, srcOffset, size);

	barrier.Swap();
	commandList.ResourceBarrier(&barrier, 1);
}

void CommandBuffer::SubmitCommandBuffer(SuperCommandBuffer *secondaryCommandBuffer)
{
	CommandBuffer *commandBuffer = InterpretAs<CommandBuffer>(secondaryCommandBuffer);
	commandList.Handle()->ExecuteBundle(*commandBuffer->GetCommandList());
}

void CommandBuffer::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	commandList.Handle()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void CommandBuffer::DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	commandList.DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandBuffer::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	commandList.Dispatch(nGroupX, nGroupY, nGroupZ);
}

void CommandBuffer::DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	commandList.DispatchMesh(nGroupX, nGroupY, nGroupZ);
}

void CommandBuffer::DispatchRays(const DeviceAddressRegion *pRayGenerationShaderRecord, const DeviceAddressRegion *pMissShaderTable, const DeviceAddressRegion *pHitGroupTable, const DeviceAddressRegion *pCallableShaderTable, uint32_t width, uint32_t height, uint32_t depth)
{
	D3D12_DISPATCH_RAYS_DESC desc = {
		.RayGenerationShaderRecord = {
		    .StartAddress = pRayGenerationShaderRecord->address,
			.SizeInBytes  = pRayGenerationShaderRecord->size,
		},
		.MissShaderTable = {
			.StartAddress  = pMissShaderTable->address,
			.SizeInBytes   = pMissShaderTable->size,
			.StrideInBytes = pMissShaderTable->stride,
		},
		.HitGroupTable = {
			.StartAddress  = pHitGroupTable->address,
			.SizeInBytes   = pHitGroupTable->size,
			.StrideInBytes = pHitGroupTable->stride,
		},
		.CallableShaderTable = {
			.StartAddress  = pCallableShaderTable->address,
			.SizeInBytes   = pCallableShaderTable->size,
			.StrideInBytes = pCallableShaderTable->stride,
		},
		.Width  = width,
		.Height = height,
		.Depth  = depth
	};
	commandList.DispatchRays(&desc);
}

}
}
