#include "IBLTask.h"
#include "FrameGraphDebugUi.h"
#include "SkyboxTask.h"

#include <imgui.h>

#include <cmath>

namespace Immortal
{

namespace
{

struct IBLIrradiancePC
{
	float deltaPhi;
	float deltaTheta;
	uint32_t outputSize;
	uint32_t pad;
};

struct IBLPrefilterPC
{
	float roughness;
	uint32_t numSamples;
	uint32_t outputSize;
	uint32_t pad;
};

static constexpr float kPi = 3.14159265358979323846f;

}

IBLTask::IBLTask() :
    RenderTask{ "IBL" }
{
}

void IBLTask::LinkSkybox(SkyboxTask *skybox)
{
	linkedSkybox = skybox;
	iblDirty = true;
}

void IBLTask::SetSourceCubemap(const Ref<Texture> &radianceCubemap)
{
	manualSource = radianceCubemap;
	iblDirty = true;
}

Ref<Texture> IBLTask::ResolveRadianceSource() const
{
	if (manualSource)
	{
		return manualSource;
	}
	if (linkedSkybox)
	{
		return linkedSkybox->GetRadianceCubemap();
	}
	return {};
}

bool IBLTask::IsIBLActive() const
{
	if (manualSource)
	{
		return true;
	}
	return linkedSkybox && linkedSkybox->GetRadianceCubemap();
}

void IBLTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
		auto device = Graphics::GetDevice();

		irradiancePipeline = Graphics::GetPipeline("ibl_irradiance");
		prefilterPipeline = Graphics::GetPipeline("ibl_prefilter");
		brdfPipeline = Graphics::GetPipeline("brdf_lut");
		if (!irradiancePipeline || !brdfPipeline)
		{
			LOG::ERR("IBLTask: {}", "failed to load ibl_irradiance / brdf_lut compute pipelines");
			return;
		}
		if (!prefilterPipeline)
		{
			LOG::WARN("IBLTask: {}", "ibl_prefilter not found; specular uses skybox mip chain.");
		}

		sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp, CompareOperation::Never, 0.0f, 16.0f);

		const TextureType iblCubeType = TextureType::Sampled | TextureType::Storage;

		blackCube = device->CreateTexture(Format::R16G16B16A16_SFLOAT, 1, 1, 1, 6, iblCubeType);
		if (blackCube)
		{
			blackCube->SetName("IBL_BlackCube");
		}

		irradianceMap = device->CreateTexture(Format::R16G16B16A16_SFLOAT, irradianceFaceSize, irradianceFaceSize, 1, 6, iblCubeType);
		if (irradianceMap)
		{
			irradianceMap->SetName("IBL_Irradiance");
		}

		brdfLut = device->CreateTexture(Format::R16G16_SFLOAT, 512, 512, 1, 1, TextureType::Sampled | TextureType::Storage);
		if (brdfLut)
		{
			brdfLut->SetName("IBL_BRDF_LUT");
		}

		irradianceDescriptorSet = device->CreateDescriptorSet(irradiancePipeline);
		brdfDescriptorSet = device->CreateDescriptorSet(brdfPipeline);
		if (prefilterPipeline)
		{
			prefilterDescriptorSet = device->CreateDescriptorSet(prefilterPipeline);
		}

		if (brdfDescriptorSet && brdfLut)
		{
			brdfDescriptorSet->Set(0, brdfLut);
			commandBuffer->SetImageLayout(brdfLut, ImageLayout::General, PipelineStage::All, PipelineStage::All);
			commandBuffer->SetPipeline(brdfPipeline);
			commandBuffer->SetDescriptorSet(brdfDescriptorSet);
			commandBuffer->Dispatch(64, 64, 1);
			commandBuffer->SetImageLayout(brdfLut, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);
		}

		if (irradianceDescriptorSet && irradianceMap && sampler)
		{
			irradianceDescriptorSet->Set(1, sampler);
			irradianceDescriptorSet->Set(2, irradianceMap);
		}
		if (prefilterDescriptorSet && sampler)
		{
			prefilterDescriptorSet->Set(1, sampler);
		}
	});
}

void IBLTask::SetIrradianceFaceSize(uint32_t size)
{
	if (size == irradianceFaceSize || size == 0)
	{
		return;
	}
	irradianceFaceSize = size;
	irradianceFaceSizeDirty = true;
}

void IBLTask::Execute(CommandBuffer *commandBuffer, const SceneParameters &)
{
	prefilterSource = ResolveRadianceSource();
	if (!prefilterSource)
	{
		prefilterSource = blackCube;
	}

	auto device = Graphics::GetDevice();
	if (!prefilterSource || !sampler || !device)
	{
		return;
	}

	if (irradianceFaceSizeDirty)
	{
		irradianceFaceSizeDirty = false;
		iblDirty = true;
		if (irradianceMap)
		{
			Graphics::ReleaseResource(irradianceMap);
		}
		const TextureType iblCubeType = TextureType::Sampled | TextureType::Storage;
		irradianceMap = device->CreateTexture(Format::R16G16B16A16_SFLOAT, irradianceFaceSize, irradianceFaceSize, 1, 6, iblCubeType);
		if (irradianceMap)
		{
			irradianceMap->SetName("IBL_Irradiance");
		}
		if (irradianceDescriptorSet)
		{
			irradianceDescriptorSet->Set(2, irradianceMap);
		}
	}

	if (!iblDirty)
	{
		return;
	}
	iblDirty = false;

	if (irradiancePipeline && irradianceDescriptorSet && irradianceMap)
	{
		commandBuffer->SetImageLayout(irradianceMap, ImageLayout::General, PipelineStage::All, PipelineStage::All);
		commandBuffer->SetImageLayout(prefilterSource, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);

		irradianceDescriptorSet->Set(0, prefilterSource);

		/* Sascha pbribl: deltaPhi = 2π/180, deltaTheta = (π/2)/64 on irradiance hemisphere grid. */
		const uint32_t phiSteps = 180u;
		const uint32_t thetaSteps = 64u;

		IBLIrradiancePC ipc{};
		ipc.deltaPhi = (2.0f * kPi) / float(phiSteps);
		ipc.deltaTheta = (0.5f * kPi) / float(thetaSteps);
		ipc.outputSize = irradianceFaceSize;

		commandBuffer->SetPipeline(irradiancePipeline);
		commandBuffer->SetDescriptorSet(irradianceDescriptorSet);
		commandBuffer->PushConstants(ShaderStage::Compute, &ipc, sizeof(ipc), 0);

		const uint32_t gx = (irradianceFaceSize + 15u) / 16u;
		const uint32_t gy = (irradianceFaceSize + 15u) / 16u;
		commandBuffer->Dispatch(gx, gy, 6);

		commandBuffer->SetImageLayout(irradianceMap, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);
	}

	if (prefilterPipeline && prefilterDescriptorSet)
	{
		commandBuffer->SetImageLayout(prefilterSource, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);

		const uint32_t faceW = prefilterSource->GetWidth();
		const uint16_t srcMips = prefilterSource->GetMipLevels();
		const bool rebuild = !prefilterBaked.Get() ||
			prefilterBaked->GetWidth() != faceW ||
			prefilterBaked->GetMipLevels() != srcMips;

		if (rebuild)
		{
			if (prefilterBaked)
			{
				Graphics::ReleaseResource(prefilterBaked);
				prefilterBaked = {};
			}
			const TextureType iblCubeType = TextureType::Sampled | TextureType::Storage;
			prefilterBaked = device->CreateTexture(Format::R16G16B16A16_SFLOAT, faceW, faceW, srcMips, 6, iblCubeType);
			if (prefilterBaked)
			{
				prefilterBaked->SetName("IBL_PrefilterRadiance");
			}
		}

		if (prefilterBaked)
		{
			commandBuffer->SetImageLayout(prefilterBaked, ImageLayout::General, PipelineStage::All, PipelineStage::All);

			prefilterDescriptorSet->Set(0, prefilterSource);

			const uint32_t mipCount = (uint32_t)std::max(1, (int)prefilterBaked->GetMipLevels());
			for (uint32_t m = 0; m < mipCount; m++)
			{
				const uint32_t dim = std::max(1u, faceW >> m);
				const float roughness = mipCount > 1 ? (float)m / (float)(mipCount - 1u) : 0.0f;

				IBLPrefilterPC ppc{};
				ppc.roughness = roughness;
				ppc.numSamples = std::max(1u, prefilterSampleCount);
				ppc.outputSize = dim;

				prefilterDescriptorSet->SetUavMip(2, prefilterBaked, m);

				commandBuffer->SetPipeline(prefilterPipeline);
				commandBuffer->SetDescriptorSet(prefilterDescriptorSet);
				commandBuffer->PushConstants(ShaderStage::Compute, &ppc, sizeof(ppc), 0);

				const uint32_t gx = (dim + 15u) / 16u;
				commandBuffer->Dispatch(gx, gx, 6);
			}

			commandBuffer->SetImageLayout(prefilterBaked, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);
		}
	}
	else if (prefilterBaked)
	{
		Graphics::ReleaseResource(prefilterBaked);
		prefilterBaked = {};
	}
}

void IBLTask::Composite(CommandBuffer *, const SceneParameters &)
{
}

void IBLTask::OnFrameGraphDebugGui()
{
	ImGui::TextUnformatted("Execute: irradiance + optional GGX prefilter + BRDF bake. Composite: none.");
	FrameGraphDebugTextureThumbnail(irradianceMap.Get(), "Irradiance (cube)");
	FrameGraphDebugTextureThumbnail(GetPrefilterRadianceMap().Get(), "Prefilter / radiance (cube)");
	FrameGraphDebugTextureThumbnail(prefilterSource.Get(), "Radiance source (cube)");
	FrameGraphDebugTextureThumbnail(brdfLut.Get(), "BRDF LUT (2D)");
}

}
