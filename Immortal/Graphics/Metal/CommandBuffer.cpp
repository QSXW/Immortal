#include "CommandBuffer.h"
#include "Device.h"
#include "Buffer.h"
#include "DescriptorSet.h"
#include "RenderTarget.h"
#include "Pipeline.h"
#include "Types.h"

namespace Immortal
{
namespace Metal
{

CommandBuffer::CommandBuffer() :
	Handle<MTL::CommandBuffer>{},
	commandQueue{},
	renderCommandEncoder{},
	computeCommandEncoder{},
	blitCommandEncoder{},
	graphicsPipeline{},
	computePipeline{},
	indexBuffer{},
	indexType{}
{

}

CommandBuffer::CommandBuffer(Device *device, MTL::CommandQueue *commandQueue) :
	Handle<MTL::CommandBuffer>{},
	commandQueue{ commandQueue },
	renderCommandEncoder{},
	computeCommandEncoder{},
	blitCommandEncoder{},
	graphicsPipeline{},
	computePipeline{},
	indexBuffer{},
	indexType{},
    indexSize{}
{

}

CommandBuffer::~CommandBuffer()
{

}

bool CommandBuffer::IsActive()
{
	return false;
}

void CommandBuffer::Reset()
{

}

void CommandBuffer::Begin()
{
    if (!handle)
    {
        handle = commandQueue->commandBufferWithUnretainedReferences();
    }
}

void CommandBuffer::End()
{
	EndCommandEncoder<MTL::BlitCommandEncoder>();
	EndCommandEncoder<MTL::ComputeCommandEncoder>();
}

void CommandBuffer::Close()
{

}

void CommandBuffer::BeginEvent(const char *pData, size_t size)
{

}

void CommandBuffer::EndEvent()
{

}

void CommandBuffer::SetPipeline(SuperPipeline *_pipeline)
{
	Pipeline *pipeline = InterpretAs<Pipeline>(_pipeline);

	auto type = pipeline->GetType();
	if (type == Pipeline::Type::Graphics)
	{
		graphicsPipeline = InterpretAs<GraphicsPipeline>(pipeline);
		renderCommandEncoder->setRenderPipelineState(*graphicsPipeline);
	}
	else if (type == Pipeline::Type::Compute)
	{
		RetriveCommandEncoder<MTL::ComputeCommandEncoder>();
		computePipeline = InterpretAs<ComputePipeline>(pipeline);
		computeCommandEncoder->setComputePipelineState(*computePipeline);
	}
}

void CommandBuffer::SetDescriptorSet(SuperDescriptorSet *_descriptorSet)
{
	DescriptorSet *descriptorSet = InterpretAs<DescriptorSet>(_descriptorSet);
	if (graphicsPipeline)
	{
		descriptorSet->Set(renderCommandEncoder);
	}
	else
	{
		RetriveCommandEncoder<MTL::ComputeCommandEncoder>();
		descriptorSet->Set(computeCommandEncoder);
	}
}

void CommandBuffer::SetVertexBuffers(uint32_t firstSlot, uint32_t bufferCount, SuperBuffer **ppBuffer, uint32_t strideInBytes)
{
	std::vector<MTL::Buffer *> buffers;
	for (size_t i = firstSlot; i < bufferCount; i++)
	{
		buffers.emplace_back(*InterpretAs<Buffer>(ppBuffer[i]));
	}

	std::vector<NS::UInteger> offsets;
	offsets.resize(bufferCount, 0);

	NS::Range range = { VertexBufferStartIndex, buffers.size() };
	renderCommandEncoder->setVertexBuffers(buffers.data(), offsets.data(), range);
}

void CommandBuffer::SetIndexBuffer(SuperBuffer *_buffer, Format format)
{
	indexBuffer = InterpretAs<Buffer>(_buffer);
	if (format == Format::R32_UINT)
	{
		indexType = MTL::IndexTypeUInt32;
        indexSize = sizeof(uint32_t);
	}
	else if (format == Format::R16_UINT)
	{
		indexType = MTL::IndexTypeUInt16;
        indexSize = sizeof(uint16_t);
	}
	else
	{
		assert(format == Format::R32_UINT || format == Format::R16_UINT);
	}
}

void CommandBuffer::SetScissors(uint32_t count, const Rect2D *pScissor)
{
	MTL::ScissorRect rect = {
		pScissor->left,
		pScissor->top,
		pScissor->right - pScissor->left,
		pScissor->bottom - pScissor->top
	};
	renderCommandEncoder->setScissorRect(rect);
}

void CommandBuffer::SetBlendFactor(const float factor[4])
{
	renderCommandEncoder->setBlendColor(factor[0], factor[1], factor[2], factor[3]);
}

void CommandBuffer::PushConstants(ShaderStage stage, const void *pData, uint32_t _size, uint32_t offset)
{
    const void *bytes = pData;
    uint8_t data[128] = {};
    uint32_t size = SLALIGN(_size, 8);
    if (size != _size)
    {
        memcpy(data, pData, _size);
        bytes = data;
    }

	switch (stage)
	{
		case ShaderStage::Vertex:
		{
			uint32_t index = graphicsPipeline->GetPushConstantIndex(PushConstantIndexType::Vertex);
			renderCommandEncoder->setVertexBytes(bytes, size, index);
			break;
		}

		case ShaderStage::Fragment:
		{
			uint32_t index = graphicsPipeline->GetPushConstantIndex(PushConstantIndexType::Fragment);
			renderCommandEncoder->setFragmentBytes(bytes, size, index);
			break;
		}

		case ShaderStage::Compute:
		{
			uint32_t index = computePipeline->GetPushConstantIndex();
			computeCommandEncoder->setBytes(bytes, size, index);
			break;
		}

		default:
			break;
	}
}

void CommandBuffer::BeginRenderTarget(SuperRenderTarget *_renderTarget, const float *pClearColor)
{
	RenderTarget *renderTarget = InterpretAs<RenderTarget>(_renderTarget);

	MTL::RenderPassDescriptor                     *renderPassDescriptor           = MTL::RenderPassDescriptor::renderPassDescriptor();
	MTL::RenderPassColorAttachmentDescriptor      *colorAttachmentDescriptor      = MTL::RenderPassColorAttachmentDescriptor::alloc()->init();
	MTL::RenderPassColorAttachmentDescriptorArray *colorAttachmentDescriptorArray = renderPassDescriptor->colorAttachments();
	auto &colorAttachments = renderTarget->InternalGetColorAttachments();
	for (size_t i = 0; i < colorAttachments.size(); i++)
	{
		MTL::ClearColor clearColor = { pClearColor[0], pClearColor[1], pClearColor[2], pClearColor[3] };
		colorAttachmentDescriptor->setClearColor(clearColor);
		colorAttachmentDescriptor->setTexture(colorAttachments[i]);
		colorAttachmentDescriptor->setLoadAction(MTL::LoadActionClear);
		colorAttachmentDescriptor->setStoreAction(MTL::StoreActionStore);
		colorAttachmentDescriptorArray->setObject(colorAttachmentDescriptor, i);
	}
	colorAttachmentDescriptor->autorelease();

	const auto &depthAttachment = renderTarget->InternalGetDepthAttachment();
	if (depthAttachment)
	{
		MTL::RenderPassDepthAttachmentDescriptor *depthAttachmentDescriptor = MTL::RenderPassDepthAttachmentDescriptor::alloc()->init();
		depthAttachmentDescriptor->setClearDepth(0);
		depthAttachmentDescriptor->setTexture(depthAttachment);
		depthAttachmentDescriptor->setLoadAction(MTL::LoadActionClear);
		depthAttachmentDescriptor->setStoreAction(MTL::StoreActionStore);
		renderPassDescriptor->setDepthAttachment(depthAttachmentDescriptor);
		depthAttachmentDescriptor->autorelease();
	}

	renderPassDescriptor->setRenderTargetWidth(renderTarget->GetWidth());
	renderPassDescriptor->setRenderTargetHeight(renderTarget->GetHeight());
	renderPassDescriptor->setRenderTargetArrayLength(colorAttachments[0].GetArrayLayers());

	EndCommandEncoder<MTL::BlitCommandEncoder>();
	EndCommandEncoder<MTL::ComputeCommandEncoder>();
	renderCommandEncoder = handle->renderCommandEncoder(renderPassDescriptor);
    renderPassDescriptor->autorelease();

	MTL::Viewport viewport = {
		.originX = 0,
		.originY = 0,
		.width   = renderTarget->GetWidth(),
		.height  = renderTarget->GetHeight(),
		.zfar    = 0.0,
		.znear   = 1.0,
	};
	renderCommandEncoder->setViewport(viewport);
    renderCommandEncoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
	renderCommandEncoder->setCullMode(MTL::CullModeNone);
}

void CommandBuffer::EndRenderTarget()
{
	renderCommandEncoder->endEncoding();
	renderCommandEncoder->autorelease();
	renderCommandEncoder = {};
}

void CommandBuffer::GenerateMipMaps(SuperTexture *_texture, Filter filter)
{
	RetriveCommandEncoder<MTL::BlitCommandEncoder>();
	Texture *texture = InterpretAs<Texture>(_texture);;
	blitCommandEncoder->generateMipmaps(*texture);
}

void CommandBuffer::CopyBufferToImage(SuperTexture *_texture, uint32_t subresource, SuperBuffer *_buffer, size_t bufferRowLength, uint32_t offset)
{
	RetriveCommandEncoder<MTL::BlitCommandEncoder>();
	Texture *texture = InterpretAs<Texture>(_texture);
	Buffer  *buffer  = InterpretAs<Buffer>(_buffer);

    MTL::Size   size   = { texture->GetWidth(), texture->GetHeight(), 1 };
	MTL::Origin origin = { 0, 0, 0 };
    blitCommandEncoder->copyFromBuffer(*buffer, offset, bufferRowLength, 0, size, *texture, 0, subresource, origin);
}

void CommandBuffer::CopyPlatformSpecificSubresource(SuperTexture *dst, uint32_t dstSubresource, void *src, uint32_t srcSubresource)
{

}

void CommandBuffer::MemoryCopy(SuperBuffer *_buffer, uint32_t size, const void *data, uint32_t offset)
{
	SLASSERT(false && "Don't call this function for Metal backend!");
}

void CommandBuffer::MemoryCopy(SuperTexture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch)
{
	SLASSERT(false && "Don't call this function for Metal backend!");
}

void CommandBuffer::SubmitCommandBuffer(SuperCommandBuffer *secondaryCommandBuffer)
{

}

void CommandBuffer::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, startVertexLocation, vertexCountPerInstance, instanceCount);
}

void CommandBuffer::DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
    renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, indexCountPerInstance, indexType, *indexBuffer, startIndexLocation * indexSize, instanceCount, baseVertexLocation, startInstanceLocation);
}

void CommandBuffer::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	RetriveCommandEncoder<MTL::ComputeCommandEncoder>();

	auto threadGroupSize = computePipeline->GetThreadGroupSize();
	MTL::Size threadsPerGrid         = { nGroupX, nGroupY, nGroupZ };
	MTL::Size threadsPerThreadgroupd = { threadGroupSize.x, threadGroupSize.y, threadGroupSize.z };
	computeCommandEncoder->dispatchThreadgroups(threadsPerGrid, threadsPerThreadgroupd);
}

void CommandBuffer::DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{

}

void CommandBuffer::DispatchRays(const DeviceAddressRegion *pRayGenerationShaderRecord, const DeviceAddressRegion *pMissShaderTable, const DeviceAddressRegion *pHitGroupTable, const DeviceAddressRegion *pCallableShaderTable, uint32_t width, uint32_t height, uint32_t depth)
{

}

void CommandBuffer::Commit()
{
	if (handle)
	{
		handle->commit();
        handle->autorelease();
		handle = {};
	}
}

}
}
