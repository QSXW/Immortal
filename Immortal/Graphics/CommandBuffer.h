#pragma once

#include "Core.h"
#include "Shared/IObject.h"
#include "Types.h"
#include "Format.h"

namespace Immortal
{

class Buffer;
class Texture;
class Pipeline;
class DescriptorSet;
class RenderTarget;
class IMMORTAL_API CommandBuffer : public IObject
{
public:
	using Type = CommandBufferType;

public:
	virtual ~CommandBuffer() = default;

	virtual bool IsActive() = 0;

	/**
	 * @brief Reset the command buffer. All of the commands recorded will be discarded.
	 */
	virtual void Reset() = 0;

	/**
	 * @brief Begin recording commands that will be submitted to the GPU
	 */
	virtual void Begin() = 0;

	/**
	 * @brief End recording
	 */
	virtual void End() = 0;

	virtual void Close() = 0;

	/**
	 * @brief For profile
	 */
	virtual void BeginEvent(const char *pData, size_t size) = 0;

	/**
	 * @brief For profile
	 */
	virtual void EndEvent() = 0;

	/**
	 * @brief Set pipeline
	 */
	virtual void SetPipeline(Pipeline *pipeline) = 0;

	virtual void SetDescriptorSet(DescriptorSet *descriptorSet) = 0;

	virtual void SetVertexBuffers(uint32_t firstSlot, uint32_t bufferCount, Buffer **ppBuffer, uint32_t strideInBytes) = 0;

	virtual void SetIndexBuffer(Buffer *buffer, Format format) = 0;

	virtual void SetScissors(uint32_t count, const Rect2D *pScissor) = 0;

	virtual void SetBlendFactor(const float factor[4]) = 0;

	virtual void PushConstants(ShaderStage stage, const void *pData, uint32_t size, uint32_t offset) = 0;

	virtual void BeginRenderTarget(RenderTarget *renderTarget, const float *pClearColor) = 0;

	virtual void EndRenderTarget() = 0;

	virtual void GenerateMipMaps(Texture *texture, Filter filter) = 0;

	virtual void CopyBufferToImage(Texture *texture, uint32_t subresource, Buffer *buffer, size_t bufferRowLength, uint32_t offset = 0) = 0;

	virtual void CopyPlatformSpecificSubresource(Texture *dst, uint32_t dstSubresource, void *src, uint32_t srcSubresource) {}

	virtual void MemoryCopy(Buffer *buffer, uint32_t size, const void *data, uint32_t offset) = 0;

	virtual void MemoryCopy(Texture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch) = 0;

	/**
	 * @brief Submit a secondary command buffer. Only support Vulkan and D3D12 backend
	 */
	virtual void SubmitCommandBuffer(CommandBuffer *secondaryCommandBuffer) = 0;

	virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) = 0;

	virtual void DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) = 0;

	virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) = 0;

	virtual void DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) = 0;

    virtual void DispatchRays(const DeviceAddressRegion *rayGenerationShaderRecord, const DeviceAddressRegion *missShaderTable, const DeviceAddressRegion *hitGroupTable, const DeviceAddressRegion *callableShaderTable, uint32_t width, uint32_t height, uint32_t depth) = 0;
};

using SuperCommandBuffer = CommandBuffer;

}
