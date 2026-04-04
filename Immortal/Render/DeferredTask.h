#pragma once

#include "Graphics.h"
#include "RenderTask.h"
#include "Graphics/RenderTarget.h"

namespace Immortal
{

class IBLTask;

/** Fullscreen lighting resolve: reads G-buffer from MeshletTask (deferred path) and blends over background. */
class DeferredTask : public RenderTask
{
public:
	DeferredTask();

	virtual ~DeferredTask() override;

	virtual void Build(AsyncComputeThread *asyncComputeThread) override;

	virtual void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	virtual void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	void SetGBufferRenderTarget(const Ref<RenderTarget> &target);

	const Ref<RenderTarget> &GetGBufferRenderTarget() const
	{
		return gbuffer;
	}

	/** When true, deferred lighting uses Cook-Torrance GGX + metallic/roughness from G-buffer (requires depth + inv VP). */
	void SetUsePBR(bool enable);

	bool GetUsePBR() const
	{
		return usePBR;
	}

	/** Optional: PBR resolve uses IBLTask outputs (t6–t8). When null, shader falls back to flat ambient. */
	void SetIBLTask(IBLTask *task)
	{
		iblTask = task;
	}

protected:
	Ref<GraphicsPipeline> pipeline;

	Ref<GraphicsPipeline> pipelinePBR;

	Ref<DescriptorSet> descriptorSet;

	Ref<DescriptorSet> descriptorSetPBR;

	Ref<Sampler> sampler;

	/** PBR resolve: cbuffer at register(b4), updated each Composite. */
	Ref<Buffer> pbrFrameBuffer;

	Ref<RenderTarget> gbuffer;

	bool usePBR = false;

	IBLTask *iblTask = nullptr;

	/** Bound to t6–t8 when IBLTask is not ready yet (avoids unbound descriptors). */
	Ref<Texture> fallbackIrradianceCube;

	Ref<Texture> fallbackRadianceCube;

	Ref<Texture> fallbackBRDF;
};

}
