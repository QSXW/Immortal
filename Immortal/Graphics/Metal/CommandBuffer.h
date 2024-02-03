#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/LightGraphics.h"
#include "Types.h"

namespace Immortal
{
namespace Metal
{

class Device;
class Buffer;
class DescriptorSet;
class Pipeline;
class GraphicsPipeline;
class ComputePipeline;
class IMMORTAL_API CommandBuffer : public SuperCommandBuffer, public Handle<MTL::CommandBuffer>
{
public:
	METAL_SWAPPABLE(CommandBuffer)

public:
	CommandBuffer();

	CommandBuffer(Device *device, MTL::CommandQueue *commandQueue);

	virtual ~CommandBuffer() override;

	virtual bool IsActive() override;

	virtual void Reset() override;

	virtual void Begin() override;

	virtual void End() override;

	virtual void Close() override;

	virtual void BeginEvent(const char *pData, size_t size) override;

	virtual void EndEvent() override;

	virtual void SetPipeline(SuperPipeline *pipeline) override;

	virtual void SetDescriptorSet(SuperDescriptorSet *descriptorSet) override;

	virtual void SetVertexBuffers(uint32_t firstSlot, uint32_t bufferCount, SuperBuffer **ppBuffer, uint32_t strideInBytes) override;

	virtual void SetIndexBuffer(SuperBuffer *buffer, Format format) override;

	virtual void SetScissors(uint32_t count, const Rect2D *pScissor) override;

	virtual void SetBlendFactor(const float factor[4]) override;

	virtual void PushConstants(ShaderStage stage, const void *pData, uint32_t size, uint32_t offset) override;

	virtual void BeginRenderTarget(SuperRenderTarget *renderTarget, const float *pClearColor) override;

	virtual void EndRenderTarget() override;

	virtual void GenerateMipMaps(SuperTexture *texture, Filter filter) override;

	virtual void CopyBufferToImage(SuperTexture *texture, uint32_t subresource, SuperBuffer *buffer, size_t bufferRowLength, uint32_t offset = 0) override;

	virtual void CopyPlatformSpecificSubresource(SuperTexture *dst, uint32_t dstSubresource, void *src, uint32_t srcSubresource) override;

	virtual void MemoryCopy(SuperBuffer *_buffer, uint32_t size, const void *data, uint32_t offset) override;

	virtual void MemoryCopy(SuperTexture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch) override;

	virtual void SubmitCommandBuffer(SuperCommandBuffer *secondaryCommandBuffer) override;

	virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;

	virtual void DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;

	virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

	virtual void DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

	virtual void DispatchRays(const DeviceAddressRegion *rayGenerationShaderRecord, const DeviceAddressRegion *missShaderTable, const DeviceAddressRegion *hitGroupTable, const DeviceAddressRegion *callableShaderTable, uint32_t width, uint32_t height, uint32_t depth) override;

public:
	void Commit();

public:
	MTL::CommandBuffer *GetCommandBuffer()
	{
		return commandQueue->commandBuffer();
	}

	template <class T>
	void RetriveCommandEncoder()
	{
		if constexpr (std::is_same_v<T, MTL::ComputeCommandEncoder>)
		{
			if (!computeCommandEncoder)
			{
				EndCommandEncoder<MTL::BlitCommandEncoder>();
				computeCommandEncoder = handle->computeCommandEncoder();
			}
		}
		else if constexpr (std::is_same_v<T, MTL::BlitCommandEncoder>)
		{
			if (!blitCommandEncoder)
			{
				EndCommandEncoder<MTL::ComputeCommandEncoder>();
				blitCommandEncoder = handle->blitCommandEncoder();
			}
		}
	}

	template <class T>
	void EndCommandEncoder()
	{
		if constexpr (std::is_same_v<T, MTL::ComputeCommandEncoder>)
		{
			computeCommandEncoder->endEncoding();
			computeCommandEncoder->autorelease();
			computeCommandEncoder = {};
		}
		else if constexpr (std::is_same_v<T, MTL::BlitCommandEncoder>)
		{
			blitCommandEncoder->endEncoding();
			blitCommandEncoder->autorelease();
			blitCommandEncoder = {};
		}
	}

	void Swap(CommandBuffer &other)
	{
		Handle::Swap(other);
		std::swap(commandQueue,          other.commandQueue         );
		std::swap(renderCommandEncoder,  other.renderCommandEncoder );
		std::swap(computeCommandEncoder, other.computeCommandEncoder);
		std::swap(blitCommandEncoder,    other.blitCommandEncoder);
		std::swap(graphicsPipeline,      other.graphicsPipeline     );
		std::swap(indexBuffer,           other.indexBuffer          );
		std::swap(indexType,             other.indexType            );
	}

protected:
	MTL::CommandQueue *commandQueue;

	MTL::RenderCommandEncoder *renderCommandEncoder;

	MTL::ComputeCommandEncoder *computeCommandEncoder;

	MTL::BlitCommandEncoder *blitCommandEncoder;

	GraphicsPipeline *graphicsPipeline;

	ComputePipeline *computePipeline;

	Buffer *indexBuffer;

	MTL::IndexType indexType;

    uint32_t indexSize;
};

}
}
