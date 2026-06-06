#include "DepthToRgbTask.h"

#include "Graphics.h"
#include "Graphics/AsyncCompute.h"
#include "Shared/Log.h"

#include <string>

namespace Immortal
{

namespace
{

struct Depth2RgbPC
{
	float rawDepthMin;
	float rawDepthMax;
};

}

DepthToRgbTask::DepthToRgbTask() :
    RenderTask{ "DepthToRgb" }
{
}

void DepthToRgbTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *) {
		auto *device = Graphics::GetDevice();

		Ref<Shader> cs{ Graphics::GetShaderByName("depth2rgb_cs", ShaderStage::Compute, "main") };
		if (!cs)
		{
			const auto path = Graphics::GetShaderAssetPath() / "depth2rgb_cs.hlsl";
			std::string src = Graphics::ReadShaderSource(path);
			if (src.empty())
			{
				LOG::ERR("DepthToRgbTask: missing depth2rgb_cs.dxil / depth2rgb_cs.hlsl under Assets/Shaders/hlsl (build ImmortalHLSL so dxc emits .dxil).");
				return;
			}
			cs = device->CreateShader("Depth2RgbCS", ShaderStage::Compute, src, "main");
		}
		if (!cs)
		{
			LOG::ERR("DepthToRgbTask: compute shader build failed");
			return;
		}

		computePipeline = device->CreateComputePipeline(cs);
		if (!computePipeline)
		{
			return;
		}

		depthSampler = device->CreateSampler(Filter::Nearest, AddressMode::Clamp);
		if (!depthSampler)
		{
			return;
		}

		computeSet = device->CreateDescriptorSet(computePipeline);
		if (computeSet)
		{
			computeSet->Set(2, depthSampler.Get());
		}

		Ref<Shader> vs{ Graphics::GetShaderByName("depth2rgb_present_VS", ShaderStage::Vertex, "VSMain") };
		Ref<Shader> ps{ Graphics::GetShaderByName("depth2rgb_present_PS", ShaderStage::Pixel, "PSMain") };
		if (!vs || !ps)
		{
			const auto path = Graphics::GetShaderAssetPath() / "depth2rgb_present.hlsl";
			std::string src = Graphics::ReadShaderSource(path);
			if (src.empty())
			{
				LOG::ERR("DepthToRgbTask: missing depth2rgb_present *_VS/_PS.dxil or depth2rgb_present.hlsl");
				return;
			}
			vs = device->CreateShader("Depth2RgbPresentVS", ShaderStage::Vertex, src, "VSMain");
			ps = device->CreateShader("Depth2RgbPresentPS", ShaderStage::Pixel, src, "PSMain");
		}
		if (!vs || !ps)
		{
			LOG::ERR("DepthToRgbTask: present shader build failed");
			return;
		}

		Shader *shaders[] = { vs, ps };
		presentPipeline = device->CreateGraphicsPipeline();
		presentPipeline->Disable(Pipeline::State::Depth);
		presentPipeline->Construct(shaders, 2, {}, { Format::RGBA8 });

		presentSampler = device->CreateSampler(Filter::Nearest, AddressMode::Clamp);
		presentSet = device->CreateDescriptorSet(presentPipeline);
		if (presentSet)
		{
			presentSet->Set(1, presentSampler.Get());
		}

		built = true;
	});
}

void DepthToRgbTask::Dispatch(CommandBuffer *commandBuffer, Texture *depth, float rawDepthMin, float rawDepthMax)
{
	if (!commandBuffer || !built || !computePipeline || !computeSet || !depth)
	{
		return;
	}

	const uint32_t w = depth->GetWidth();
	const uint32_t h = depth->GetHeight();
	if (w == 0u || h == 0u)
	{
		return;
	}

	auto *device = Graphics::GetDevice();
	if (!outputRgb || outputRgb->GetWidth() != w || outputRgb->GetHeight() != h)
	{
		if (outputRgb)
		{
			Graphics::ReleaseResource(outputRgb);
		}
		outputRgb = device->CreateTexture(
		    Format::RGBA8,
		    w,
		    h,
		    1,
		    1,
		    TextureType::Sampled | TextureType::Storage);
		if (outputRgb)
		{
			outputRgb->SetName("DepthToRgbOutput");
			computeSet->Set(1, outputRgb.Get());
			computeSet->Set(2, depthSampler.Get());
		}
	}

	if (!outputRgb)
	{
		return;
	}
	
	commandBuffer->BeginEvent(name);
	computeSet->Set(0, depth);
	computeSet->Set(1, outputRgb.Get());
	computeSet->Set(2, depthSampler.Get());

	Depth2RgbPC pc{ rawDepthMin, rawDepthMax };

	commandBuffer->SetImageLayout(outputRgb.Get(), ImageLayout::General, PipelineStage::All, PipelineStage::All);
	commandBuffer->SetPipeline(computePipeline);
	commandBuffer->SetDescriptorSet(computeSet);
	commandBuffer->PushConstants(ShaderStage::Compute, &pc, sizeof(pc), 0u);
	commandBuffer->Dispatch((w + 7u) / 8u, (h + 7u) / 8u, 1u);
	commandBuffer->SetImageLayout(outputRgb.Get(), ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);

	commandBuffer->EndEvent();
}

void DepthToRgbTask::DrawPresentFullscreen(CommandBuffer *commandBuffer)
{
	if (!commandBuffer || !built || !presentPipeline || !presentSet || !outputRgb)
	{
		return;
	}

	presentSet->Set(0, outputRgb.Get());
	commandBuffer->SetPipeline(presentPipeline);
	commandBuffer->SetDescriptorSet(presentSet);
	commandBuffer->DrawInstanced(3, 1, 0, 0);
}

}
