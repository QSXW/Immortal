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

static bool HasStage(PipelineStage stages, PipelineStage stage)
{
	return !!(stages & stage);
}

static D3D12_RESOURCE_STATES CastShaderResourceState(D3D12_COMMAND_LIST_TYPE commandListType, PipelineStage stage)
{
	if (commandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
	{
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}

	const bool computeOnly =
		HasStage(stage, PipelineStage::ComputeShading) &&
		!HasStage(stage, PipelineStage::PixelShading) &&
		!HasStage(stage, PipelineStage::Draw) &&
		!HasStage(stage, PipelineStage::RenderTarget) &&
		!HasStage(stage, PipelineStage::All) &&
		!HasStage(stage, PipelineStage::AllShading);
	if (computeOnly)
	{
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}

	return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
}

static D3D12_RESOURCE_STATES SanitizeResourceStateForCommandList(D3D12_RESOURCE_STATES state, D3D12_COMMAND_LIST_TYPE commandListType)
{
	if (commandListType != D3D12_COMMAND_LIST_TYPE_COMPUTE)
	{
		return state;
	}

	if (state & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
	{
		state = (D3D12_RESOURCE_STATES)(state & ~D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		state = (D3D12_RESOURCE_STATES)(state | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	return state;
}

static D3D12_RESOURCE_STATES CAST(ImageLayout layout, D3D12_COMMAND_LIST_TYPE commandListType, PipelineStage stage = PipelineStage::All)
{
	switch (layout)
	{
		case ImageLayout::Undefined:
		case ImageLayout::General:
			return D3D12_RESOURCE_STATE_COMMON;

		case ImageLayout::Present:
			return D3D12_RESOURCE_STATE_PRESENT;

		case ImageLayout::GenericRead:
			return D3D12_RESOURCE_STATE_GENERIC_READ;

		case ImageLayout::RenderTarget:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;

		case ImageLayout::UnorderedAccess:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		case ImageLayout::DepthStencilWrite:
			return D3D12_RESOURCE_STATE_DEPTH_WRITE;

		case ImageLayout::DepthStencilRead:
			return D3D12_RESOURCE_STATE_DEPTH_READ;

		case ImageLayout::ShaderResource:
			return CastShaderResourceState(commandListType, stage);

		case ImageLayout::TransferSource:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;

		case ImageLayout::TransferDestination:
			return D3D12_RESOURCE_STATE_COPY_DEST;

		case ImageLayout::ResolveSource:
			return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

		case ImageLayout::ResolveDestination:
			return D3D12_RESOURCE_STATE_RESOLVE_DEST;

		case ImageLayout::ShadingRateSource:
			return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;

		case ImageLayout::VideoDecodeRead:
			return D3D12_RESOURCE_STATE_VIDEO_DECODE_READ;

		case ImageLayout::VideoDecodeWrite:
			return D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;

		case ImageLayout::VideoProcessRead:
			return D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ;

		case ImageLayout::VideoProcessWrite:
			return D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE;

		case ImageLayout::VideoEncodeRead:
			return D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ;

		case ImageLayout::VideoEncodeWrite:
			return D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;

		default:
			return D3D12_RESOURCE_STATE_COMMON;
	}
}

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
	commandListType{ type },
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
	ID3D12PipelineState *pipelineState = *computePipeline;
	if (pipelineState)
	{
		commandList.SetPipelineState(pipelineState);
	}
	else
	{
		ComPtr<ID3D12GraphicsCommandList10> commandList10;
		DX_CHECK(commandList.QueryInterface(commandList10.GetAddressOf()));

		D3D12_SET_PROGRAM_DESC desc = computePipeline->GetSetProgramDesc();
		commandList10->SetProgram(&desc);
	}

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
	activeBarrier = 0;
	renderTarget  = nullptr;
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
	std::wstring label = std::filesystem::path(pData).wstring();
	commandList.Handle()->BeginEvent(0, label.c_str(), (label.size() + 1) * sizeof(wchar_t));
}

void CommandBuffer::EndEvent()
{
	commandList.Handle()->EndEvent();
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
	if (pipeline && pipeline->GetType() == Pipeline::Type::Compute)
	{
		const auto &bindings = descriptorSet->GetTextureBindings();
		for (size_t i = 0; i < bindings.size(); i++)
		{
			Texture *texture = bindings[i].texture;
			if (!texture)
			{
				continue;
			}

			bool firstBinding = true;
			bool unorderedAccess = false;
			for (size_t j = 0; j < bindings.size(); j++)
			{
				if (bindings[j].texture != texture)
				{
					continue;
				}
				if (j < i)
				{
					firstBinding = false;
				}
				unorderedAccess |= bindings[j].rangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			}

			if (!firstBinding)
			{
				continue;
			}
			if (unorderedAccess)
			{
				texture->WaitLockRelease();
			}

			SetImageLayout(
				texture,
				unorderedAccess ? ImageLayout::UnorderedAccess : ImageLayout::ShaderResource,
				PipelineStage::All,
				PipelineStage::ComputeShading,
				nullptr);
		}
	}

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
	if (stage & ShaderStage::Compute)
	{
		commandList.PushComputeConstant(size, pData, offset);
	}
	else
	{
		if (stage & ShaderStage::Vertex)
		{
			commandList.PushGraphicsConstant(size, pData, offset, pipeline->GetPushConstantRootParameterIndex(D3D12_SHADER_VISIBILITY_VERTEX));
		}
		if (stage & ShaderStage::Pixel)
		{
			commandList.PushGraphicsConstant(size, pData, offset, pipeline->GetPushConstantRootParameterIndex(D3D12_SHADER_VISIBILITY_PIXEL));
		}
		if (stage & ShaderStage::Mesh)
		{
			commandList.PushGraphicsConstant(size, pData, offset, pipeline->GetPushConstantRootParameterIndex(D3D12_SHADER_VISIBILITY_MESH));
		}
	}
}

void CommandBuffer::BeginRenderTarget(SuperRenderTarget *_renderTarget, const ClearValue *pClearValue)
{
	renderTarget = InterpretAs<RenderTarget>(_renderTarget);
	if (!renderTarget)
	{
		return;
	}

	auto &colorBuffers = renderTarget->GetColorBuffers();
	Texture *depthBuffer  = renderTarget->GetDepthBuffer();
	if (colorBuffers.empty() && !depthBuffer)
	{
		renderTarget = nullptr;
		return;
	}

	if (depthBuffer && (!*depthBuffer || !renderTarget->GetDepthBufferDescriptor().ptr))
	{
		renderTarget = nullptr;
		return;
	}

	auto &rtvDescriptor = renderTarget->GetDescriptor();
	const auto &dsvDescriptor = renderTarget->GetDepthBufferDescriptor();
	if (!colorBuffers.empty() && !rtvDescriptor)
	{
		renderTarget = nullptr;
		return;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle[32];
	const D3D12_CPU_DESCRIPTOR_HANDLE *dstDescriptorHandle = nullptr;

	for (size_t i = 0; i < colorBuffers.size(); i++)
	{
		if (!colorBuffers[i] || !*colorBuffers[i])
		{
			renderTarget = nullptr;
			return;
		}
		rtvDescriptorHandle[i] = rtvDescriptor[i];
		if (!rtvDescriptorHandle[i].ptr)
		{
			renderTarget = nullptr;
			return;
		}
	}
	if (dsvDescriptor.ptr)
	{
		dstDescriptorHandle = &dsvDescriptor;
	}

	const uint32_t width = !colorBuffers.empty() ? colorBuffers[0]->GetWidth() : depthBuffer->GetWidth();
	const uint32_t height = !colorBuffers.empty() ? colorBuffers[0]->GetHeight() : depthBuffer->GetHeight();
	if (width == 0 || height == 0)
	{
		renderTarget = nullptr;
		return;
	}

	Viewport viewport{ 0, 0, width, height };
	commandList.RSSetViewports(&viewport);

	D3D12_RECT scissorRect{ 0, 0, (LONG)width, (LONG)height };
	commandList.RSSetScissorRects(&scissorRect);

	for (auto &colorBuffer : colorBuffers)
	{
		barriers[activeBarrier++].Transition(
		    *colorBuffer,
		    colorBuffer->GetState(),
		    D3D12_RESOURCE_STATE_RENDER_TARGET);
		colorBuffer->SetState(D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	if (depthBuffer)
	{
		barriers[activeBarrier++].Transition(
		    *depthBuffer,
		    depthBuffer->GetState(),
		    D3D12_RESOURCE_STATE_DEPTH_WRITE);
		depthBuffer->SetState(D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	commandList.ResourceBarrier(barriers.data(), activeBarrier);

	if (pClearValue)
	{
		for (size_t i = 0; i < colorBuffers.size(); i++)
		{
			commandList.ClearRenderTargetView(rtvDescriptorHandle[i], (float *)&pClearValue[i]);
		}

		if (dsvDescriptor.ptr)
		{
			auto &clearValue = pClearValue[colorBuffers.size()];
			commandList.ClearDepthStencilView(*dstDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH, clearValue.depthStencil.depth, clearValue.depthStencil.stencil);
		}
	}
	commandList.SetRenderTargets(rtvDescriptorHandle, uint32_t(colorBuffers.size()), false, dstDescriptorHandle);
}

void CommandBuffer::EndRenderTarget()
{
	if (!renderTarget)
	{
		activeBarrier = 0;
		return;
	}

	for (uint32_t i = 0; i < activeBarrier; i++)
	{
		barriers[i].Swap();
	}
	commandList.ResourceBarrier(barriers.data(), activeBarrier);

	auto &colorBuffers = renderTarget->GetColorBuffers();
	uint32_t barrierIndex = 0;
	for (auto &colorBuffer : colorBuffers)
	{
		const auto &barrier = static_cast<const D3D12_RESOURCE_BARRIER &>(barriers[barrierIndex++]);
		colorBuffer->SetState(barrier.Transition.StateAfter);
	}
	if (Texture *depthBuffer = renderTarget->GetDepthBuffer())
	{
		const auto &barrier = static_cast<const D3D12_RESOURCE_BARRIER &>(barriers[barrierIndex]);
		depthBuffer->SetState(barrier.Transition.StateAfter);
	}

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
	uint32_t arrayLayer = texture->GetArrayLayers();

	if (mipLevels <= 1)
	{
		return;
	}

	if (arrayLayer > 1 && arrayLayer != 6)
	{
		return;
	}

	Sampler *sampler = device->GetSampler(filter);
	const char *name = arrayLayer == 1 ? "GenerateMipMaps" : "GenerateMipMapsCube";
	Pipeline *pipeline = device->GetPipeline(name);

	if (!pipeline)
	{
		return;
	}

	SuperCommandBuffer::BeginEvent(name);

	descriptorSets.reserve(descriptorSets.size() + mipLevels);
	const D3D12_RESOURCE_STATES previousState = SanitizeResourceStateForCommandList(texture->GetState(), commandListType);
	if (previousState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	{
		Barrier<BarrierType::Transition> barrier{ *texture, previousState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS };
		commandList.ResourceBarrier(&barrier);
	}
	texture->SetState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		URef<DescriptorSet>	descriptorSet = new DescriptorSet{ device, pipeline };
		uint32_t width  = std::max(texture->GetWidth() >> i, 1u);
		uint32_t height = std::max(texture->GetHeight() >> i, 1u);
		descriptorSet->Set(0, sampler);
		descriptorSet->Set(0, texture->GetDescriptor(i - 1), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
		descriptorSet->Set(1, texture->GetUAVDescriptor(i),  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);

		SetPipeline(pipeline);
		SetDescriptorSet(descriptorSet);
		descriptorSets.emplace_back(std::move(descriptorSet));

		float ratio[] = {
		    1.0f / width,
		    1.0f / height,
		};

		PushConstants(ShaderStage::Compute, ratio, sizeof(ratio), 0);
		Dispatch(std::max(SLALIGN(width / 8, 8), 1u), std::max(SLALIGN(height / 8, 8), 1u), arrayLayer);

		Barrier<BarrierType::UAV> barrier{ *texture };
		commandList.ResourceBarrier(&barrier);
	}
	Barrier<BarrierType::Transition> toCommon{ *texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON };
	commandList.ResourceBarrier(&toCommon);
	texture->SetState(D3D12_RESOURCE_STATE_COMMON);

	EndEvent();
}

void CommandBuffer::CopyTextureRegion(SuperTexture *_texture, uint32_t subresource, uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t z, SuperBuffer *_buffer, size_t bufferRowLength, uint32_t offset)
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
	            .Width    = width,
	            .Height   = height,
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

	const D3D12_RESOURCE_STATES previousState = SanitizeResourceStateForCommandList(texture->GetState(), commandListType);
	if (previousState != D3D12_RESOURCE_STATE_COPY_DEST)
	{
		Barrier<BarrierType::Transition> barrier{
			*texture,
			previousState,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		};
		commandList.ResourceBarrier(&barrier, 1);
	}
	texture->SetState(D3D12_RESOURCE_STATE_COPY_DEST);

	commandList.CopyTextureRegion(&dstLocation, x, y, z, &srcLocation, nullptr);
	Barrier<BarrierType::Transition> toCommon{
		*texture,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	commandList.ResourceBarrier(&toCommon, 1);
	texture->SetState(D3D12_RESOURCE_STATE_COMMON);
}

void CommandBuffer::CopyBufferToImage(SuperTexture *texture, uint32_t subresource, SuperBuffer *buffer, size_t bufferRowLength, uint32_t offset)
{
	return CopyTextureRegion(texture, subresource, texture->GetWidth(), texture->GetHeight(), 0, 0, 0, buffer, bufferRowLength, offset);
}

void CommandBuffer::CopyImageToBuffer(SuperBuffer *_buffer, SuperTexture *_texture, uint32_t subresource, size_t bufferRowLength, const Rect2D *pRect)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	Buffer *buffer   = InterpretAs<Buffer>(_buffer);

	Format format = texture->SuperTexture::GetFormat();
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

	const D3D12_RESOURCE_STATES previousState = SanitizeResourceStateForCommandList(texture->GetState(), commandListType);
	if (previousState != D3D12_RESOURCE_STATE_COPY_SOURCE)
	{
		Barrier<BarrierType::Transition> barrier{
			*texture,
			previousState,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		};
		commandList.ResourceBarrier(&barrier, 1);
	}
	texture->SetState(D3D12_RESOURCE_STATE_COPY_SOURCE);

	D3D12_BOX box;
	D3D12_BOX *pBox = nullptr;
	if (pRect)
	{
		box = D3D12_BOX{
		    .left   = pRect->left,
		    .top    = pRect->top,
		    .front  = 0,
		    .right  = pRect->right,
		    .bottom = pRect->bottom,
		    .back   = 1,
		};

		auto &footprint = dstLocation.PlacedFootprint.Footprint;
		footprint.Width  = pRect->right - pRect->left;
		footprint.Height = pRect->bottom - pRect->top;
		pBox = &box;
	}

	commandList.CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, pBox);
	Barrier<BarrierType::Transition> toCommon{
		*texture,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	commandList.ResourceBarrier(&toCommon, 1);
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

	const D3D12_RESOURCE_STATES previousState = SanitizeResourceStateForCommandList(texture->GetState(), commandListType);
	Barrier<Transition> toCopy[2] = {};
	uint32_t toCopyCount = 0;
	toCopy[toCopyCount++].Transition(
		srcLocation.pResource,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		srcSubresource);
	if (previousState != D3D12_RESOURCE_STATE_COPY_DEST)
	{
		toCopy[toCopyCount++].Transition(
			dstLocation.pResource,
			previousState,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	}
	commandList.ResourceBarrier(toCopy, toCopyCount);
	texture->SetState(D3D12_RESOURCE_STATE_COPY_DEST);

	commandList.CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

	Barrier<Transition> toCommon[] = {
		{ srcLocation.pResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON, srcSubresource },
		{ dstLocation.pResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES },
	};
	commandList.ResourceBarrier(toCommon, SL_ARRAY_LENGTH(toCommon));
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

void CommandBuffer::Memset(SuperBuffer *_buffer, const ClearValue *pClearValue, Format format)
{
	Buffer *buffer = InterpretAs<Buffer>(_buffer);

	auto type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	URef<DescriptorSet> descriptorSet = new DescriptorSet{device, 1, type};

	auto srcDescriptor = buffer->GetDescriptor();
	auto dstDescriptor = descriptorSet->GetDescriptors(type);
	device->CopyDescriptors(1, dstDescriptor.descriptor, srcDescriptor, type);

	DescriptorHeap *descriptorHeap = descriptorSet->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ID3D12DescriptorHeap *ppDescriptorHeap[2] = { *descriptorHeap };
	commandList.SetDescriptorHeaps(ppDescriptorHeap, 1);

	Barrier<Transition> barrier(*buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList.ResourceBarrier(&barrier, 1);
	if (format == Format::UINT32)
	{
		commandList.Handle()->ClearUnorderedAccessViewUint(
		    dstDescriptor.shaderVisibleDescriptor,
		    srcDescriptor,
		    *buffer,
		    (const UINT *)pClearValue,
		    0,
		    nullptr);
	}
	else
	{
		commandList.Handle()->ClearUnorderedAccessViewFloat(
		    dstDescriptor.shaderVisibleDescriptor,
		    srcDescriptor,
		    *buffer,
		    (const FLOAT *)pClearValue,
		    0,
		    nullptr);
	}

	barrier.Swap();
	commandList.ResourceBarrier(&barrier, 1);
	descriptorSets.emplace_back(std::move(descriptorSet));
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

void CommandBuffer::MemoryBarrier(SuperTexture *_texture)
{
	if (!_texture)
	{
		return;
	}
	Texture *texture = InterpretAs<Texture>(_texture);
	// UAV barrier: ensures every subsequent compute dispatch observes the
	// writes performed by previous dispatches on this resource. Without
	// this between successive compositor passes, a pixel a layer reads
	// from `Base` may still hold the previous frame's value (or
	// uninitialized memory on a freshly created texture), which manifests
	// as the green-screen / old-frame flicker users see during playback.
	Barrier<BarrierType::UAV> barrier{ *texture };
	commandList.ResourceBarrier(&barrier);
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

void CommandBuffer::DispatchGraph(const DispatchGraphDescription *pDesc)
{
	D3D12_DISPATCH_GRAPH_DESC desc = {
		.Mode         = (D3D12_DISPATCH_MODE) pDesc->mode,
		.NodeCPUInput = (D3D12_NODE_CPU_INPUT &)pDesc->nodeCpuInput
	};

	ComPtr<ID3D12GraphicsCommandList10> commandList10;
	DX_CHECK(commandList.QueryInterface(commandList10.GetAddressOf()));
	commandList10->DispatchGraph(&desc);
}

void CommandBuffer::SetImageLayout(SuperTexture *_texture, ImageLayout layout, PipelineStage from, PipelineStage to, const SubresourceRange *pSubresourceRange)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	D3D12_RESOURCE_STATES oldState = SanitizeResourceStateForCommandList(texture->GetState(), commandListType);
	D3D12_RESOURCE_STATES newState = SanitizeResourceStateForCommandList(CAST(layout, commandListType, to), commandListType);

	if (newState == oldState)
	{
		texture->SetState(newState);
		return;
	}

	Barrier<BarrierType::Transition> barrier{
	    *texture,
	    oldState,
	    newState,
	    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};

	commandList.ResourceBarrier(&barrier, 1);
	texture->SetState(newState);
}

void CommandBuffer::SetShaderResource(uint32_t slot, GpuVirtualAddress address)
{
	const D3D12_DESCRIPTOR_RANGE_TYPE *rangeTypes = pipeline->GetDescriptorRangeType();
	auto &descriptorTables = pipeline->GetDescriptorTables();
	uint32_t index = descriptorTables[slot].RootParameterIndex;

	switch (rangeTypes[slot])
	{
		case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
			commandList.Handle()->SetGraphicsRootConstantBufferView(index, address);
			break;

		case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
			commandList.Handle()->SetGraphicsRootShaderResourceView(index, address);
			break;

		case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
			commandList.Handle()->SetGraphicsRootUnorderedAccessView(index, address);
			break;

		default:
			break;
	}
}

void CommandBuffer::ResolveImage(SuperTexture *_dst, SuperTexture *_src)
{
	Texture *dst = InterpretAs<Texture>(_dst);
	Texture *src = InterpretAs<Texture>(_src);

	const D3D12_RESOURCE_STATES srcBefore = src->GetState();
	const D3D12_RESOURCE_STATES dstBefore = dst->GetState();

	Barrier<BarrierType::Transition> toResolve[2] = {};
	uint32_t n = 0;
	if (srcBefore != D3D12_RESOURCE_STATE_RESOLVE_SOURCE)
	{
		toResolve[n++].Transition(*src, srcBefore, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	}
	if (dstBefore != D3D12_RESOURCE_STATE_RESOLVE_DEST)
	{
		toResolve[n++].Transition(*dst, dstBefore, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	}
	if (n)
	{
		commandList.Handle()->ResourceBarrier(n, toResolve);
	}
	commandList.Handle()->ResolveSubresource(*dst, 0, *src, 0, dst->GetFormat());

	Barrier<BarrierType::Transition> fromResolve[] = {
		{ *src, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_COMMON },
		{ *dst, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON },
	};
	commandList.Handle()->ResourceBarrier(SL_ARRAY_LENGTH(fromResolve), fromResolve);
	src->SetState(D3D12_RESOURCE_STATE_COMMON);
	dst->SetState(D3D12_RESOURCE_STATE_COMMON);
}

void CommandBuffer::CopyTexture(SuperTexture *_dst, SuperTexture *_src)
{
	Texture *dst = InterpretAs<Texture>(_dst);
	Texture *src = InterpretAs<Texture>(_src);
	if (!dst || !src || dst->GetWidth() != src->GetWidth() || dst->GetHeight() != src->GetHeight())
	{
		return;
	}

	Barrier<BarrierType::UAV> uavFlush{ *src };
	commandList.ResourceBarrier(&uavFlush, 1);

	Barrier<BarrierType::Transition> toCopy[2] = {};
	uint32_t nToCopy = 0;
	const D3D12_RESOURCE_STATES src0 = src->GetState();
	const D3D12_RESOURCE_STATES dst0 = dst->GetState();
	if (src0 != D3D12_RESOURCE_STATE_COPY_SOURCE)
	{
		toCopy[nToCopy++].Transition(*src, src0, D3D12_RESOURCE_STATE_COPY_SOURCE);
	}
	if (dst0 != D3D12_RESOURCE_STATE_COPY_DEST)
	{
		toCopy[nToCopy++].Transition(*dst, dst0, D3D12_RESOURCE_STATE_COPY_DEST);
	}
	if (nToCopy)
	{
		commandList.ResourceBarrier(toCopy, nToCopy);
	}
	src->SetState(D3D12_RESOURCE_STATE_COPY_SOURCE);
	dst->SetState(D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12_TEXTURE_COPY_LOCATION dstLoc = { .pResource = *dst, .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, .SubresourceIndex = 0 };
	D3D12_TEXTURE_COPY_LOCATION srcLoc = { .pResource = *src, .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, .SubresourceIndex = 0 };
	commandList.CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

	Barrier<BarrierType::Transition> toCommon[2] = {};
	uint32_t nToCommon = 0;
	if (src->GetState() != D3D12_RESOURCE_STATE_COMMON)
	{
		toCommon[nToCommon++].Transition(*src, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	}
	if (dst->GetState() != D3D12_RESOURCE_STATE_COMMON)
	{
		toCommon[nToCommon++].Transition(*dst, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
	}
	if (nToCommon)
	{
		commandList.ResourceBarrier(toCommon, nToCommon);
	}
	src->SetState(D3D12_RESOURCE_STATE_COMMON);
	dst->SetState(D3D12_RESOURCE_STATE_COMMON);
}

}
}
