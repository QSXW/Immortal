#pragma once

#include "Graphics/CommandBuffer.h"
#include "Common.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class Buffer;
class Pipeline;
class IMMORTAL_API CommandBuffer : public SuperCommandBuffer, public NonDispatchableHandle
{
public:
	using Primitive = ID3D11DeviceContext4;
	D3D11_OPERATOR_HANDLE()
	D3D11_SWAPPABLE(CommandBuffer)

public:
	CommandBuffer(Device *device = nullptr);

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

	virtual void MemoryCopy(SuperBuffer *buffer, uint32_t size, const void *data, uint32_t offset) override;

	virtual void MemoryCopy(SuperTexture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch) override;

	virtual void SubmitCommandBuffer(SuperCommandBuffer *secondaryCommandBuffer) override;

	virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;

	virtual void DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;

	virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

	virtual void DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

	virtual void DispatchRays(const DeviceAddressRegion *rayGenerationShaderRecord, const DeviceAddressRegion *missShaderTable, const DeviceAddressRegion *hitGroupTable, const DeviceAddressRegion *callableShaderTable, uint32_t width, uint32_t height, uint32_t depth) override;

public:
	template <class T>
	void Submit(T &&command)
	{
		command();
	}

	void Swap(CommandBuffer &other)
	{
		NonDispatchableHandle::Swap(other);
		std::swap(handle,   other.handle  );
		std::swap(pipeline, other.pipeline);
	}

protected:
	Pipeline *pipeline;

	URef<Buffer> pushConstantBuffer;
};

}
}
