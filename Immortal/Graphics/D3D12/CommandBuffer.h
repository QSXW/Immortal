#pragma once

#include "Common.h"
#include "Graphics/CommandBuffer.h"
#include "Handle.h"
#include "CommandList.h"
#include "CommandAllocator.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class DescriptorSet;
class CommandAllocatorPool;
class CommandList;
class Pipeline;
class GraphicsPipeline;
class ComputePipeline;
class RenderTarget;
class IMMORTAL_API CommandBuffer : public SuperCommandBuffer, public NonDispatchableHandle
{
public:
	CommandBuffer(Device *device, D3D12_COMMAND_LIST_TYPE type);

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

	virtual void CopyImageToBuffer(SuperBuffer *buffer, SuperTexture *texture, uint32_t subresource, size_t bufferRowLength) override;

	virtual void CopyPlatformSpecificSubresource(SuperTexture *dst, uint32_t dstSubresource, void *src, uint32_t srcSubresource) override;

	virtual void MemoryCopy(SuperBuffer *_buffer, uint32_t size, const void *data, uint32_t offset) override;

	virtual void MemoryCopy(SuperTexture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch) override;

	virtual void MemoryCopy(SuperBuffer *dst, uint32_t dstOffset, SuperBuffer *src, uint32_t srcOffset, size_t size) override;

	virtual void SubmitCommandBuffer(SuperCommandBuffer *secondaryCommandBuffer) override;

	virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;

	virtual void DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;

	virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

	virtual void DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

	virtual void DispatchRays(const DeviceAddressRegion *rayGenerationShaderRecord, const DeviceAddressRegion *missShaderTable, const DeviceAddressRegion *hitGroupTable, const DeviceAddressRegion *callableShaderTable, uint32_t width, uint32_t height, uint32_t depth) override;

public:
	void SetGraphicsPipeline(GraphicsPipeline *graphicsPipeline);

	void SetComputePipeline(ComputePipeline *computePipeline);

public:
	CommandList *GetCommandList()
	{
		return &commandList;
	}

protected:
	CommandAllocatorPool allocatorPool;

	CommandList commandList;

	std::array<Barrier<BarrierType::Transition>, 8> barriers;

	uint32_t activeBarrier;

    CommandAllocator *allocator;

	Pipeline *pipeline;

	std::vector<URef<DescriptorSet>> descriptorSets;

	using PFN_SetRootDescriptorTable = void (*)(CommandList *, uint32_t, D3D12_GPU_DESCRIPTOR_HANDLE);
	PFN_SetRootDescriptorTable SetRootDescriptorTable;

	RenderTarget *renderTarget;
};

}
}
