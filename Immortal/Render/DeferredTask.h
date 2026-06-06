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

	void OnFrameGraphDebugGui() override;

	void SetGBufferRenderTarget(const Ref<RenderTarget> &target);

	const Ref<RenderTarget> &GetGBufferRenderTarget() const
	{
		return gbuffer;
	}

	/** SceneLightingModel index (Unlit, Phong, PBR, NPR) — selects fullscreen resolve pipeline. */
	void SetResolveLightingMode(uint32_t mode);

	uint32_t GetResolveLightingMode() const
	{
		return resolveLightingMode;
	}

	/** Optional: PBR resolve uses IBLTask outputs (t6–t8). When null, shader falls back to flat ambient. */
	void SetIBLTask(IBLTask *task)
	{
		iblTask = task;
	}

	/** Directional shadow: one depth (`d0`) for single-map; `d0`–`d3` for four cascades (CSM). */
	void SetShadowMapForResolve(
	    Texture *d0,
	    Texture *d1,
	    Texture *d2,
	    Texture *d3,
	    const Matrix4 &lightViewProjection,
	    const Matrix4 *cascadeViewProjection,
	    const float *cascadeSplits,
	    uint32_t cascadeCount,
	    bool enabled,
	    float bias,
	    float strength,
	    uint32_t casterLightIndex = 0);

	struct NPRParams
	{
		Vector3 shadowColor{ 0.55f, 0.35f, 0.45f };
		float shadowThreshold = 0.15f;
		float shadowSoftness  = 0.04f;
		float litIntensity    = 1.0f;
		float rimPower        = 4.0f;
		float rimIntensity    = 0.35f;
		float edgeDepthThreshold  = 0.008f;
		float edgeNormalThreshold = 0.8f;
		float edgeIntensity       = 0.5f;
	};

	NPRParams &GetNPRParams() { return nprParams; }
	const NPRParams &GetNPRParams() const { return nprParams; }

protected:
	static constexpr uint32_t kResolvePipelineCount = 4;

	Ref<GraphicsPipeline> resolvePipelines[kResolvePipelineCount];

	Ref<DescriptorSet> resolveDescriptorSets[kResolvePipelineCount];

	Ref<Sampler> sampler;

	/** Depth comparisons must not use linear filtering on the shadow map. */
	Ref<Sampler> samplerShadowPoint;

	/** PBR resolve: cbuffer at register(b4), updated each Composite. */
	Ref<Buffer> pbrFrameBuffer;

	Ref<Buffer> nprFrameBuffer;

	/** Simple deferred resolve: cbuffer register(b4). */
	Ref<Buffer> simpleFrameBuffer;

	Ref<RenderTarget> gbuffer;

	uint32_t resolveLightingMode = 0;

	NPRParams nprParams;

	IBLTask *iblTask = nullptr;

	/** Bound to t6–t8 when IBLTask is not ready yet (avoids unbound descriptors). */
	Ref<Texture> fallbackIrradianceCube;

	Ref<Texture> fallbackRadianceCube;

	Ref<Texture> fallbackBRDF;

	Texture *shadowResolveDepths[4]{};

	Matrix4 shadowResolveLightVP{};

	Matrix4 shadowResolveCascadeVP[SceneParameters::MaxShadowCascades]{};

	float shadowResolveCascadeSplits[SceneParameters::MaxShadowCascades]{};

	uint32_t shadowResolveCascadeCount = 0;

	bool shadowResolveEnabled = false;

	float shadowResolveBias = 0.0025f;

	float shadowResolveStrength = 1.0f;

	uint32_t shadowResolveCasterLightIndex = 0;

	Ref<RenderTarget> fallbackShadowDepthRT;
};

}
