#pragma once

#include "Graphics.h"
#include "Mesh.h"
#include "RenderTask.h"
#include "Shared/IObject.h"
#include "String/IString.h"
#include "Scene/Component.h"

namespace Immortal
{

class IBLTask;

struct SceneConstantBuffer
{
	Matrix4  World;
	Matrix4  WorldView;
	Matrix4  WorldViewProj;
	uint32_t DrawMeshlets;
};

struct MeshInfo
{
	Matrix4 Model;
	uint32_t IndexBytes;
	uint32_t MeshletOffset;
};

enum class MeshletRenderPath : uint32_t
{
	Forward = 0,
	Deferred = 1,
};

/** Matches `SceneLightingModel` indices: Unlit, Phong, PBR, NPR. G-buffer pass uses a separate pipeline slot. */
enum class MeshletPipelineSlot : uint32_t
{
	Unlit = 0,
	Phong = 1,
	PBR = 2,
	NPR = 3,
	GBuffer = 4,
	Count = 5,
};

class MeshletTask : public RenderTask
{
public:
	MeshletTask();

	~MeshletTask() override;

	void Build(AsyncComputeThread *asyncComputeThread) override;

	void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	void DrawMesh(
	    CommandBuffer *commandBuffer,
	    const SceneParameters &params,
	    uint32_t objectId,
	    const TransformComponent &transform,
	    MeshComponent &meshComponent,
	    const MaterialComponent &materialComponent,
	    const RenderPass *pass = nullptr) override;

	void OnFrameGraphDebugGui() override;

	void SetShadowMaps(Texture *d0, Texture *d1, Texture *d2, Texture *d3, Texture *shadowDepthRgbViz = nullptr);

	/** Forward PBR: irradiance + prefiltered spec + BRDF LUT (same split-sum as deferred resolve). */
	void SetFrameEnvironmentMaps(Texture *irradiance, Texture *prefilter, Texture *brdfLut, uint32_t useIBL, float maxSpecularLod);

	void SetIBLTask(IBLTask *ibl);

	/** Forward vs deferred for skinned mesh path (meshlet_skinned.hlsl). Static path uses pass flags. */
	void SetRenderPath(MeshletRenderPath path)
	{
		renderPath = path;
	}

	MeshletRenderPath GetRenderPath() const
	{
		return renderPath;
	}

protected:
	static constexpr uint32_t kPipelineCount = (uint32_t)MeshletPipelineSlot::Count;

	Ref<GraphicsPipeline> pipelines[kPipelineCount];
	Ref<GraphicsPipeline> skinPipelines[kPipelineCount];

	Ref<Sampler> sampler;
	Ref<Sampler> shadowSamplerClamp;
	Ref<Sampler> shadowPassRgbSampler;
	Ref<Sampler> iblEnvSamplerClamp;

	Ref<Buffer> stagingBuffer;
	Ref<Buffer> constantBuffer;
	Ref<Buffer> sceneBuffer;
	Ref<Buffer> cascadeBuffer;
	Ref<Buffer> sceneStaging;
	Ref<Buffer> cascadeStaging;

	Ref<RenderTarget> fallbackShadowDepthRT;
	Ref<RenderTarget> fallbackShadowPassRgbRT;

	Texture *shadowDepths[4]{};
	Texture *shadowDepthRgbViz = nullptr;

	Texture *frameIrradianceCube = nullptr;
	Texture *framePrefilterCube = nullptr;
	Texture *frameBrdfLut = nullptr;
	uint32_t frameUseIBL = 0;
	float frameMaxSpecularLod = 0.0f;

	IBLTask *linkedIbl = nullptr;

	MeshletRenderPath renderPath = MeshletRenderPath::Forward;
};

}
