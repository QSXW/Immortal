#pragma once

#include "Graphics.h"
#include "RenderTask.h"

namespace Immortal
{

/** Bloom + filmic tonemap + gamma: reads HDR scene (R16) + pick, writes LDR scene output. */
class PostProcessTask : public RenderTask
{
public:
	PostProcessTask();

	~PostProcessTask() override = default;

	void Build(AsyncComputeThread *asyncComputeThread) override;

	void Execute(CommandBuffer *, const SceneParameters &) override {}

	void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	void SetHdrSceneColor(Texture *hdrColor);

	void SetHdrPickTexture(Texture *pickTexture);

	void SetViewportSize(uint32_t width, uint32_t height);

protected:
	Ref<GraphicsPipeline> pipeline;
	Ref<DescriptorSet> descriptorSet;
	Ref<Sampler> linearSampler;
	Ref<Buffer> constantBuffer;

	Texture *hdrColorTex{ nullptr };
	Texture *hdrPickTex{ nullptr };
	uint32_t viewportW{ 0 };
	uint32_t viewportH{ 0 };
};

}
