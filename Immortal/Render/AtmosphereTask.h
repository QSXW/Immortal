#pragma once

#include "RenderTask.h"
#include "Graphics.h"
#include "Mesh.h"

namespace Immortal
{

enum class AtmosphereQuality : uint32_t
{
	Low  = 0,
	High = 1
};

/** Optional procedural sky (Rayleigh / Mie). Add to FrameGraph like SkyboxTask; use one or the other as background. */
class AtmosphereTask : public RenderTask
{
public:
	AtmosphereTask();

	virtual ~AtmosphereTask() override;

	virtual void Build(AsyncComputeThread *asyncComputeThread) override;

	virtual void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	virtual void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	void OnFrameGraphDebugGui() override;

	void SetEnabled(bool value)
	{
		enabled = value;
	}

	bool GetEnabled() const
	{
		return enabled;
	}

	void SetQuality(AtmosphereQuality value)
	{
		quality = value;
	}

	AtmosphereQuality GetQuality() const
	{
		return quality;
	}

	/** World-space direction toward the sun (need not be normalized). */
	void SetSunDirection(const Vector3 &direction);

	const Vector3 &GetSunDirection() const
	{
		return sunDirection;
	}

	void SetSunIntensity(float value)
	{
		sunIntensity = value;
	}

	float GetSunIntensity() const
	{
		return sunIntensity;
	}

	/** Seconds (or any monotonic time) for shader animation / day gradient. */
	void SetDayPhase(float seconds)
	{
		dayPhase = seconds;
	}

	float GetDayPhase() const
	{
		return dayPhase;
	}

protected:
	bool enabled = true;

	AtmosphereQuality quality = AtmosphereQuality::High;

	Vector3 sunDirection{ 0.35f, 0.85f, 0.25f };

	float sunIntensity = 22.0f;

	float dayPhase = 0.0f;

	Ref<Mesh> skybox;

	Ref<DescriptorSet> descriptorSet;

	Ref<Sampler> sampler;

	Ref<Texture> dummyWhite;
};

}
