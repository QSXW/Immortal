#include "UiPresentScale.h"

#include "Log.h"
#include "Render/Graphics.h"

namespace Immortal
{

bool UiPresentScale::Build(Device *device)
{
	if (!device)
	{
		return false;
	}

	Ref<Shader> cs{ Graphics::GetShaderByName("ui_present_scale", ShaderStage::Compute, "main") };
	if (!cs)
	{
		LOG::WARN("UiPresentScale: missing ui_present_scale.dxil / ui_present_scale.hlsl (build ImmortalHLSL).");
		return false;
	}

	computePipeline = device->CreateComputePipeline(cs);
	if (!computePipeline)
	{
		return false;
	}

	linearSampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);
	if (!linearSampler)
	{
		return false;
	}

	computeSet = device->CreateDescriptorSet(computePipeline);
	built = computeSet != nullptr;
	return built;
}

void UiPresentScale::EnsureScratch(Device *device, uint32_t dstWidth, uint32_t dstHeight)
{
	if (!device || dstWidth == 0 || dstHeight == 0)
	{
		return;
	}

	if (scratch && scratchW == dstWidth && scratchH == dstHeight)
	{
		return;
	}

	scratch.Reset();
	scratchW = dstWidth;
	scratchH = dstHeight;
	scratch = device->CreateTexture(Format::BGRA8, dstWidth, dstHeight, 1, 1, TextureType::Storage);
}

void UiPresentScale::Composite(CommandBuffer *commandBuffer, Texture *internalColor, Texture *swapchainColor, uint32_t dstWidth, uint32_t dstHeight)
{
	if (!commandBuffer || !internalColor || !swapchainColor || !scratch || !IsReady() || dstWidth == 0 || dstHeight == 0)
	{
		return;
	}

	struct Push
	{
		uint32_t dstW;
		uint32_t dstH;
		uint32_t pad0;
		uint32_t pad1;
	} push{ dstWidth, dstHeight, 0, 0 };

	commandBuffer->BeginEvent("UiPresentScale::Composite");

	commandBuffer->SetImageLayout(internalColor, ImageLayout::ShaderResource, PipelineStage::RenderTarget, PipelineStage::ComputeShading);
	commandBuffer->SetImageLayout(scratch.Get(), ImageLayout::UnorderedAccess, PipelineStage::All, PipelineStage::ComputeShading);

	computeSet->Set(0, internalColor);
	computeSet->Set(1, scratch);
	computeSet->Set(2, linearSampler.Get());

	commandBuffer->SetPipeline(computePipeline.Get());
	commandBuffer->SetDescriptorSet(computeSet.Get());
	commandBuffer->PushConstants(ShaderStage::Compute, &push, sizeof(push), 0);
	commandBuffer->Dispatch((dstWidth + 7u) / 8u, (dstHeight + 7u) / 8u, 1u);

	commandBuffer->CopyTexture(swapchainColor, scratch.Get());

	commandBuffer->SetImageLayout(internalColor, ImageLayout::General, PipelineStage::ComputeShading, PipelineStage::All);

	commandBuffer->EndEvent();
}

}
