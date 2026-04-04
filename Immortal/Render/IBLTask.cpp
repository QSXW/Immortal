#include "IBLTask.h"
#include "SkyboxTask.h"

namespace Immortal
{

namespace
{

struct IBLIrradiancePC
{
	uint32_t faceSize;
	uint32_t sampleCount;
	uint32_t pad[2];
};

}

IBLTask::IBLTask() :
    RenderTask{ "IBL" }
{
}

void IBLTask::LinkSkybox(SkyboxTask *skybox)
{
	linkedSkybox = skybox;
}

void IBLTask::SetSourceCubemap(const Ref<Texture> &radianceCubemap)
{
	manualSource = radianceCubemap;
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
	asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t, CommandBuffer *commandBuffer) {
		auto device = Graphics::GetDevice();

		irradiancePipeline = Graphics::GetPipeline("ibl_irradiance");
		brdfPipeline = Graphics::GetPipeline("brdf_lut");
		if (!irradiancePipeline || !brdfPipeline)
		{
			LOG::ERR("IBLTask: failed to load ibl_irradiance / brdf_lut compute pipelines");
			return;
		}

		sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);

		blackCube = device->CreateTexture(Format::R16G16B16A16_SFLOAT, 1, 1, 1, 6, TextureType::Storage);
		if (blackCube)
		{
			blackCube->SetName("IBL_BlackCube");
		}

		irradianceMap = device->CreateTexture(Format::R16G16B16A16_SFLOAT, irradianceFaceSize, irradianceFaceSize, 1, 6, TextureType::Storage);
		if (irradianceMap)
		{
			irradianceMap->SetName("IBL_Irradiance");
		}

		brdfLut = device->CreateTexture(Format::R16G16_SFLOAT, 512, 512, 1, 1, TextureType::Storage);
		if (brdfLut)
		{
			brdfLut->SetName("IBL_BRDF_LUT");
		}

		irradianceDescriptorSet = device->CreateDescriptorSet(irradiancePipeline);
		brdfDescriptorSet = device->CreateDescriptorSet(brdfPipeline);

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
	});
}

void IBLTask::Execute(CommandBuffer *commandBuffer, const SceneParameters &)
{
	prefilterSource = ResolveRadianceSource();
	if (!prefilterSource)
	{
		prefilterSource = blackCube;
	}

	if (!irradiancePipeline || !irradianceDescriptorSet || !irradianceMap || !prefilterSource || !sampler)
	{
		return;
	}

	commandBuffer->SetImageLayout(irradianceMap, ImageLayout::General, PipelineStage::All, PipelineStage::All);
	commandBuffer->SetImageLayout(prefilterSource, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);

	irradianceDescriptorSet->Set(0, prefilterSource);

	IBLIrradiancePC pc{};
	pc.faceSize = irradianceFaceSize;
	pc.sampleCount = irradianceSampleCount;

	commandBuffer->SetPipeline(irradiancePipeline);
	commandBuffer->SetDescriptorSet(irradianceDescriptorSet);
	commandBuffer->PushConstants(ShaderStage::Compute, &pc, sizeof(pc), 0);

	uint32_t gx = (irradianceFaceSize + 15u) / 16u;
	uint32_t gy = (irradianceFaceSize + 15u) / 16u;
	commandBuffer->Dispatch(gx, gy, 6);

	commandBuffer->SetImageLayout(irradianceMap, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);
}

void IBLTask::Composite(CommandBuffer *, const SceneParameters &)
{
}

}
