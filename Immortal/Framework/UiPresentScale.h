#pragma once

#include "Core.h"
#include "Graphics/LightGraphics.h"
#include "Shared/IObject.h"

namespace Immortal
{

/** Compute down/up-scale of UI color (BGRA8) to swapchain resolution. */
class UiPresentScale
{
public:
	bool Build(Device *device);

	/** Pipeline/sampler ready; scratch is created in EnsureScratch (RebuildUiCompositeTargets), not in Build. */
	bool IsReady() const
	{
		return built && computePipeline && computeSet && linearSampler;
	}

	void EnsureScratch(Device *device, uint32_t dstWidth, uint32_t dstHeight);

	void Composite(CommandBuffer *commandBuffer, Texture *internalColor, Texture *swapchainColor, uint32_t dstWidth, uint32_t dstHeight);

private:
	bool built = false;
	Ref<Pipeline> computePipeline;
	Ref<DescriptorSet> computeSet;
	Ref<Sampler> linearSampler;
	Ref<Texture> scratch;
	uint32_t scratchW = 0;
	uint32_t scratchH = 0;
};

}
