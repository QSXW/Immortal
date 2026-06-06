#include "DeferredTask.h"
#include "IBLTask.h"

#include <cstring>
#include <algorithm>

namespace Immortal
{

namespace
{

struct DeferredLightGPU
{
	Vector4 direction;
	Vector4 radiance;
};

struct DeferredResolveSimpleConstants
{
	DeferredLightGPU lights[4];
};

struct DeferredResolvePBRFrameConstants
{
	Matrix4 invViewProjection;
	Vector4 cameraWorld;
	DeferredLightGPU lights[4];
	uint32_t lightCount;
	uint32_t useIBL;
	float maxSpecularLod;
	float padFrame;
};

}

DeferredTask::DeferredTask() :
    RenderTask{ "DeferredLighting" }
{
}

DeferredTask::~DeferredTask()
{
}

void DeferredTask::SetUsePBR(bool enable)
{
	usePBR = enable;
}

void DeferredTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t, CommandBuffer *) {
		auto device = Graphics::GetDevice();
		fallbackIrradianceCube = device->CreateTexture(Format::R16G16B16A16_SFLOAT, 1, 1, 1, 6, TextureType::Storage);
		fallbackRadianceCube = device->CreateTexture(Format::R16G16B16A16_SFLOAT, 1, 1, 1, 6, TextureType::Storage);
		fallbackBRDF = device->CreateTexture(Format::R16G16_SFLOAT, 1, 1, 1, 1, TextureType::Storage);

		std::string src = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "deferred_resolve.hlsl");
		if (src.empty())
		{
			LOG::ERR("DeferredTask: failed to read deferred_resolve.hlsl");
			return;
		}
		Ref<Shader> vs = device->CreateShader("DeferredVS", ShaderStage::Vertex, src, "VSMain");
		Ref<Shader> ps = device->CreateShader("DeferredPS", ShaderStage::Pixel, src, "PSMain");
		if (!vs || !ps)
		{
			LOG::ERR("DeferredTask: shader compile failed");
			return;
		}
		Shader *shaders[] = { vs, ps };
		pipeline = device->CreateGraphicsPipeline();
		pipeline->Disable(Pipeline::State::Depth);
		pipeline->Construct(shaders, 2, {}, { Format::R8G8B8A8_UNORM });

		sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);
		descriptorSet = device->CreateDescriptorSet(pipeline);
		descriptorSet->Set(2, sampler);

		std::string srcPbr = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "deferred_resolve_pbr.hlsl");
		if (!srcPbr.empty())
		{
			Ref<Shader> vsPbr = device->CreateShader("DeferredVSPBR", ShaderStage::Vertex, srcPbr, "VSMain");
			Ref<Shader> psPbr = device->CreateShader("DeferredPSPBR", ShaderStage::Pixel, srcPbr, "PSMain");
			if (vsPbr && psPbr)
			{
				Shader *shadersPbr[] = { vsPbr, psPbr };
				pipelinePBR = device->CreateGraphicsPipeline();
				pipelinePBR->Disable(Pipeline::State::Depth);
				pipelinePBR->Construct(shadersPbr, 2, {}, { Format::R8G8B8A8_UNORM });
				descriptorSetPBR = device->CreateDescriptorSet(pipelinePBR);
				descriptorSetPBR->Set(3, sampler);
				pbrFrameBuffer = device->CreateBuffer(BufferType::ConstantBuffer, 256, MemoryType::Host);
				descriptorSetPBR->Set(4, pbrFrameBuffer);
			}
			else
			{
				LOG::ERR("DeferredTask: PBR shader compile failed");
			}
		}
	});
}

void DeferredTask::Execute(CommandBuffer *, const SceneParameters &)
{
}

void DeferredTask::SetGBufferRenderTarget(const Ref<RenderTarget> &target)
{
	gbuffer = target;
}

void DeferredTask::Composite(CommandBuffer *commandBuffer, const SceneParameters &params)
{
	if (!gbuffer)
	{
		return;
	}

	Texture *alb = gbuffer->GetColorAttachment(0);
	Texture *nrm = gbuffer->GetColorAttachment(1);
	if (!alb || !nrm)
	{
		return;
	}

	if (usePBR && pipelinePBR && descriptorSetPBR && pbrFrameBuffer)
	{
		Texture *depth = gbuffer->GetDepthAttachment();
		if (!depth)
		{
			return;
		}

		descriptorSetPBR->Set(0, alb);
		descriptorSetPBR->Set(1, nrm);
		descriptorSetPBR->Set(2, depth);

		DeferredResolvePBRFrameConstants frame{};
		frame.invViewProjection = params.invViewProjection;
		frame.cameraWorld = params.cameraWorld;
		if (params.lightCount == 0)
		{
			frame.lights[0].direction = Vector4{ 0.35f, 0.85f, 0.25f, 0.0f };
			frame.lights[0].radiance = Vector4{ 4.0f, 4.0f, 4.0f, 0.0f };
			frame.lightCount = 1;
		}
		else
		{
			frame.lightCount = params.lightCount;
			for (uint32_t i = 0; i < params.lightCount; i++)
			{
				frame.lights[i].direction = params.lights[i].direction;
				frame.lights[i].radiance = params.lights[i].radiance;
			}
		}

		frame.useIBL = 0;
		frame.maxSpecularLod = 0.0f;
		frame.padFrame = 0.0f;

		Ref<Texture> irr = fallbackIrradianceCube;
		Ref<Texture> pre = fallbackRadianceCube;
		Ref<Texture> brdf = fallbackBRDF;
		if (iblTask)
		{
			if (iblTask->GetIrradianceMap())
			{
				irr = iblTask->GetIrradianceMap();
			}
			if (iblTask->GetPrefilterRadianceMap())
			{
				pre = iblTask->GetPrefilterRadianceMap();
			}
			if (iblTask->GetBRDFLUT())
			{
				brdf = iblTask->GetBRDFLUT();
			}
			frame.useIBL = iblTask->IsIBLActive() ? 1u : 0u;
			if (iblTask->GetPrefilterRadianceMap())
			{
				uint32_t mips = (uint32_t)std::max(1, (int)iblTask->GetPrefilterRadianceMap()->GetMipLevels());
				frame.maxSpecularLod = (float)std::max(0, (int)mips - 1);
			}
		}
		descriptorSetPBR->Set(6, irr);
		descriptorSetPBR->Set(7, pre);
		descriptorSetPBR->Set(8, brdf);

		void *mapped = nullptr;
		pbrFrameBuffer->Map(&mapped, 256, 0);
		std::memcpy(mapped, &frame, sizeof(frame));
		std::memset(static_cast<uint8_t *>(mapped) + sizeof(frame), 0, 256 - sizeof(frame));
		pbrFrameBuffer->Unmap();

		commandBuffer->SetPipeline(pipelinePBR);
		commandBuffer->SetDescriptorSet(descriptorSetPBR);
		commandBuffer->DrawInstanced(3, 1, 0, 0);
		return;
	}

	if (!pipeline || !descriptorSet)
	{
		return;
	}

	descriptorSet->Set(0, alb);
	descriptorSet->Set(1, nrm);

	DeferredResolveSimpleConstants pc{};
	if (params.lightCount == 0)
	{
		pc.lights[0].direction = Vector4{ 0.35f, 0.85f, 0.25f, 0.0f };
		pc.lights[0].radiance = Vector4{ 1.0f, 1.0f, 1.0f, 0.0f };
	}
	else
	{
		for (uint32_t i = 0; i < params.lightCount; i++)
		{
			pc.lights[i].direction = params.lights[i].direction;
			pc.lights[i].radiance = params.lights[i].radiance;
		}
	}

	commandBuffer->SetPipeline(pipeline);
	commandBuffer->SetDescriptorSet(descriptorSet);
	commandBuffer->PushConstants(ShaderStage::Pixel, &pc, sizeof(pc), 0);
	commandBuffer->DrawInstanced(3, 1, 0, 0);
}

}
