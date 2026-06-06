#pragma once

#include "Graphics.h"
#include "RenderTask.h"

namespace Immortal
{

/** Compute: depth texture → RGBA (for ImGui / debug). Optional fullscreen present pass. */
class DepthToRgbTask : public RenderTask
{
public:
	DepthToRgbTask();

	~DepthToRgbTask() override = default;

	void Build(AsyncComputeThread *asyncComputeThread) override;

	void Execute(CommandBuffer *, const SceneParameters &) override {}

	void Composite(CommandBuffer *, const SceneParameters &) override {}

	/** Runs compute: writes internal RGBA target (same size as `depth`). */
	void Dispatch(CommandBuffer *commandBuffer, Texture *depth, float rawDepthMin, float rawDepthMax);

	/** Fullscreen into current color RT (expects `Dispatch` already ran this frame). */
	void DrawPresentFullscreen(CommandBuffer *commandBuffer);

	bool IsReady() const
	{
		return built && computePipeline && computeSet && presentPipeline && presentSet;
	}

	Texture *GetOutputTexture() const
	{
		return outputRgb.Get();
	}

private:
	bool built = false;
	Ref<Pipeline> computePipeline;
	Ref<DescriptorSet> computeSet;
	Ref<Sampler> depthSampler;
	Ref<Texture> outputRgb;

	Ref<GraphicsPipeline> presentPipeline;
	Ref<DescriptorSet> presentSet;
	Ref<Sampler> presentSampler;
};

}
