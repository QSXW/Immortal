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

	void OnFrameGraphDebugGui() override;

	/** Optional: sample radiance from SkyboxTask::GetRadianceCubemap() each frame. */
	void LinkSkybox(SkyboxTask *skybox);

	/** Direct radiance cubemap (e.g. baked atmosphere or custom HDR cube). Overrides LinkSkybox when set. */
	void SetSourceCubemap(const Ref<Texture> &radianceCubemap);

	void MarkDirty() { iblDirty = true; }

	const Ref<Texture> &GetIrradianceMap() const
	{
		return irradianceMap;
	}

	/** Specular term samples this cubemap with LOD = roughness * MaxSpecularLod (split-sum; mips from ibl_prefilter when available). */
	const Ref<Texture> &GetPrefilterRadianceMap() const
	{
		return prefilterBaked.Get() ? prefilterBaked : prefilterSource;
	}

	const Ref<Texture> &GetBRDFLUT() const
	{
		return brdfLut;
	}

	bool IsIBLActive() const;

	void SetIrradianceFaceSize(uint32_t size);

	uint32_t GetIrradianceFaceSize() const
	{
		return irradianceFaceSize;
	}

	void SetIrradianceSampleCount(uint32_t count)
	{
		irradianceSampleCount = count;
	}

	uint32_t GetIrradianceSampleCount() const
	{
		return irradianceSampleCount;
	}

private:
	Ref<Texture> ResolveRadianceSource() const;

	SkyboxTask *linkedSkybox = nullptr;

	Ref<Texture> manualSource;

	Ref<Texture> blackCube;

	Ref<Texture> irradianceMap;

	Ref<Texture> brdfLut;

	Ref<Texture> prefilterSource;

	Ref<Texture> prefilterBaked;

	Ref<Pipeline> irradiancePipeline;

	Ref<Pipeline> prefilterPipeline;

	Ref<Pipeline> brdfPipeline;

	Ref<DescriptorSet> irradianceDescriptorSet;

	Ref<DescriptorSet> prefilterDescriptorSet;

	static constexpr uint32_t kMaxPrefilterMips = 16;
	Ref<DescriptorSet> prefilterMipDescriptorSets[kMaxPrefilterMips];

	Ref<DescriptorSet> brdfDescriptorSet;

	Ref<Sampler> sampler;

	uint32_t irradianceFaceSize = 64;

	/** Deprecated for dispatch: irradiance uses fixed 180×64 grid (Sascha pbribl). Kept for API compatibility. */
	uint32_t irradianceSampleCount = 11520;

	uint32_t prefilterSampleCount = 1024;

	bool irradianceFaceSizeDirty = false;

	bool iblDirty = true;
};

}
