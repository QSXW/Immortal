#pragma once

#include "Core.h"
#include "Shared/IObject.h"
#include "Types.h"
#include "Format.h"

#include <memory>

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

	/** Vulkan: vkCmdSetDepthBias. D3D12/Metal/OpenGL: optional no-op (bias may be baked in PSO). */
	virtual void SetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
	{
		(void)depthBiasConstantFactor;
		(void)depthBiasClamp;
		(void)depthBiasSlopeFactor;
	}

	virtual void PushConstants(ShaderStage stage, const void *pData, uint32_t size, uint32_t offset) = 0;

	virtual void BeginRenderTarget(RenderTarget *renderTarget, const ClearValue *pClearValues) = 0;

	virtual void EndRenderTarget() = 0;

	virtual void GenerateMipMaps(Texture *texture, Filter filter) = 0;

	virtual void CopyTextureRegion(Texture *texture, uint32_t subresource, uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t z, Buffer *buffer, size_t bufferRowLength, uint32_t offset = 0) {}

	virtual void CopyBufferToImage(Texture *texture, uint32_t subresource, Buffer *buffer, size_t bufferRowLength, uint32_t offset = 0) = 0;

	virtual void CopyImageToBuffer(Buffer *buffer, Texture *texture, uint32_t subresource, size_t bufferRowLength, const Rect2D *pRect = nullptr) {}

	virtual void CopyPlatformSpecificSubresource(Texture *dst, uint32_t dstSubresource, void *src, uint32_t srcSubresource) {}

	virtual void MemoryCopy(Buffer *buffer, uint32_t size, const void *data, uint32_t offset) = 0;

	virtual void MemoryCopy(Texture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch) = 0;

	virtual void MemoryCopy(Buffer *dst, uint32_t dstOffset, Buffer *src, uint32_t srcOffset, size_t size) {}

	virtual void Memset(Buffer *buffer, const ClearValue *pClearValue, Format format)
	{

	}

	/**
	 * @brief Submit a secondary command buffer. Only support Vulkan and D3D12 backend
	 */
	virtual void SubmitCommandBuffer(CommandBuffer *secondaryCommandBuffer) = 0;

	virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) = 0;

	virtual void DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) = 0;

	virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) = 0;

	virtual void DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) = 0;

    virtual void DispatchRays(const DeviceAddressRegion *rayGenerationShaderRecord, const DeviceAddressRegion *missShaderTable, const DeviceAddressRegion *hitGroupTable, const DeviceAddressRegion *callableShaderTable, uint32_t width, uint32_t height, uint32_t depth) = 0;

	virtual void DispatchGraph(const DispatchGraphDescription *pDesc)
	{

	}

	virtual void SetImageLayout(Texture *texture, ImageLayout layout, PipelineStage from, PipelineStage to, const SubresourceRange *pSubresourceRange = std::addressof(kAllSubresources))
	{

	}
	
	virtual void SetShaderResource(uint32_t slot, GpuVirtualAddress address)
	{

	}

	virtual void ResolveImage(Texture *dst, Texture *src)
	{

	}

	/** Full-subresource color copy; textures must match in format, size, mips, and layers. */
	virtual void CopyTexture(Texture *dst, Texture *src)
	{
		(void)dst;
		(void)src;
	}

	/**
	 * @brief Insert a UAV (unordered-access) memory barrier so subsequent
	 *        dispatches/draws see the writes from previous dispatches.
	 *
	 * Required between successive compute dispatches that write the same
	 * UAV resource (e.g. a multi-pass post-processing chain or a per-layer
	 * compositor). Backends that already serialize compute work (D3D11,
	 * single-queue OpenGL) can leave the default no-op.
	 */
	virtual void MemoryBarrier(Texture *texture)
	{
		(void)texture;
	}

	void BeginEvent(const std::string &event)
	{
		BeginEvent(event.c_str(), event.size() + 1);
	}
};

using SuperCommandBuffer = CommandBuffer;

}
