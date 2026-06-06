#pragma once

#include "RenderTask.h"
#include "Graphics.h"

namespace Immortal
{

class SkyboxTask;

class IBLTask : public RenderTask
{
public:
	IBLTask();

	virtual ~IBLTask() override = default;

	virtual void Build(AsyncComputeThread *asyncComputeThread) override;

	virtual void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	virtual void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	/** Optional: sample radiance from SkyboxTask::GetRadianceCubemap() each frame. */
	void LinkSkybox(SkyboxTask *skybox);

	/** Direct radiance cubemap (e.g. baked atmosphere or custom HDR cube). Overrides LinkSkybox when set. */
	void SetSourceCubemap(const Ref<Texture> &radianceCubemap);

	const Ref<Texture> &GetIrradianceMap() const
	{
		return irradianceMap;
	}

	/** Specular term samples this cubemap with LOD = roughness * MaxSpecularLod (split-sum approximation). */
	const Ref<Texture> &GetPrefilterRadianceMap() const
	{
		return prefilterSource;
	}

	const Ref<Texture> &GetBRDFLUT() const
	{
		return brdfLut;
	}

	bool IsIBLActive() const;

private:
	Ref<Texture> ResolveRadianceSource() const;

	SkyboxTask *linkedSkybox = nullptr;

	Ref<Texture> manualSource;

	Ref<Texture> blackCube;

	Ref<Texture> irradianceMap;

	Ref<Texture> brdfLut;

	Ref<Texture> prefilterSource;

	Ref<Pipeline> irradiancePipeline;

	Ref<Pipeline> brdfPipeline;

	Ref<DescriptorSet> irradianceDescriptorSet;

	Ref<DescriptorSet> brdfDescriptorSet;

	Ref<Sampler> sampler;

	uint32_t irradianceFaceSize = 32;

	uint32_t irradianceSampleCount = 64;
};

}
