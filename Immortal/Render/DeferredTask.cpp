#include "DeferredTask.h"
#include "FrameGraphDebugUi.h"
#include "IBLTask.h"
#include "Graphics/Types.h"
#include "Render/Graphics.h"

#include <imgui.h>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <cfloat>
#include <filesystem>

namespace Immortal
{

namespace
{

static constexpr uint32_t kDeferredFrameBufferSize = 1024;

struct DeferredLightSimpleGPU
{
	Vector4 direction;
	Vector4 radiance;
};

struct DeferredLightGPU
{
	Vector4 direction;
	Vector4 radiance;
	Vector4 position;
	Vector4 spotParams;
};

struct DeferredResolveSimpleFrameConstants
{
	Matrix4 invViewProjection;
	Vector4 cameraWorld;
	Matrix4 view;
	DeferredLightSimpleGPU lights[4];
	uint32_t lightCount;
	uint32_t shadowEnabled;
	float shadowBias;
	float shadowStrength;
	uint32_t shadowCasterLightIndex;
	Matrix4 shadowViewProjection;
	Matrix4 shadowCascadeViewProjection[4];
	float shadowCascadeSplits[4];
	uint32_t shadowCascadeCount;
	uint32_t _padShadowCascade[3];
};

struct DeferredResolvePBRFrameConstants
{
	Matrix4 invViewProjection;
	Vector4 cameraWorld;
	Matrix4 view;
	DeferredLightGPU lights[4];
	uint32_t lightCount;
	uint32_t useIBL;
	float maxSpecularLod;
	float exposure;
	float gamma;
	float _pad[3];
	Matrix4 shadowViewProjection;
	Matrix4 shadowCascadeViewProjection[4];
	float shadowCascadeSplits[4];
	uint32_t shadowCascadeCount;
	float _shadowCascadePad[3];
	uint32_t shadowEnabled;
	float shadowBias;
	float shadowStrength;
	uint32_t shadowCasterLightIndex;
};

struct DeferredResolveNPRConstants
{
	Matrix4 invViewProjection;
	Vector4 cameraWorld;
	Matrix4 view;
	DeferredLightGPU lights[4];
	uint32_t lightCount;
	float shadowThreshold;
	float shadowSoftness;
	float litIntensity;
	float shadowColor[3];
	float rimPower;
	float rimIntensity;
	float edgeDepthThreshold;
	float edgeNormalThreshold;
	float edgeIntensity;
	float exposure;
	float gamma;
	float nprPadBeforeShadowMatrix[2];
	Matrix4 shadowViewProjection;
	Matrix4 shadowCascadeViewProjection[4];
	float shadowCascadeSplits[4];
	uint32_t shadowCascadeCount;
	float _shadowCascadePad[3];
	uint32_t shadowEnabled;
	float shadowBias;
	float shadowStrength;
	uint32_t shadowCasterLightIndex;
};

static_assert(sizeof(DeferredLightGPU) == 64);

}

DeferredTask::DeferredTask() :
    RenderTask{ "DeferredLighting" }
{
}

DeferredTask::~DeferredTask()
{
}

void DeferredTask::SetResolveLightingMode(uint32_t mode)
{
	resolveLightingMode = mode > 3u ? 0u : mode;
}

void DeferredTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
		auto device = Graphics::GetDevice();
		fallbackIrradianceCube = device->CreateTexture(Format::R16G16B16A16_SFLOAT, 1, 1, 1, 6, TextureType::Storage);
		fallbackRadianceCube = device->CreateTexture(Format::R16G16B16A16_SFLOAT, 1, 1, 1, 6, TextureType::Storage);
		fallbackBRDF = device->CreateTexture(Format::R16G16_SFLOAT, 1, 1, 1, 1, TextureType::Storage);

		ClearValue shadowDepthClear[1] = { { .depthStencil = { 1.0f, 0 } } };
		fallbackShadowDepthRT = device->CreateRenderTarget(1, 1, nullptr, 0, Format::Depth24Stencil8, shadowDepthClear);

		std::string srcGBuffer;
		bool sourceLoadAttempted = false;
		auto loadOrCompile = [&](ShaderStage st, const std::string &dxilName, const char *entry, const ShaderMacro *macros = nullptr, uint32_t numMacro = 0) -> Ref<Shader> {
			const auto dxilPath = Graphics::GetShaderAssetPath() / (dxilName + ".dxil");
			if (device->GetBackendAPI() == BackendAPI::D3D12 && std::filesystem::exists(dxilPath))
			{
				if (Shader *shader = Graphics::CreateShaderFromDXIL(device, dxilPath, st))
				{
					return Ref<Shader>{ shader };
				}
			}
			if (!sourceLoadAttempted)
			{
				srcGBuffer = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "gbuffer_screen_resolve.hlsl");
				sourceLoadAttempted = true;
			}
			if (srcGBuffer.empty())
			{
				return {};
			}
			return device->CreateShader(dxilName, st, srcGBuffer, std::string(entry), macros, numMacro);
		};

		static const ShaderMacro kGBufferVariantPhong[] = { { "IMMORTAL_GBUFFER_SCREEN_VARIANT", "0" } };
		static const ShaderMacro kGBufferVariantPbr[]  = { { "IMMORTAL_GBUFFER_SCREEN_VARIANT", "1" } };
		static const ShaderMacro kGBufferVariantNpr[]  = { { "IMMORTAL_GBUFFER_SCREEN_VARIANT", "2" } };

		Ref<Shader> vs  = loadOrCompile(ShaderStage::Vertex, "gbuffer_screen_resolve_VS", "VSMain", kGBufferVariantPhong, 1u);
		Ref<Shader> ps  = loadOrCompile(ShaderStage::Pixel, "gbuffer_screen_resolve_PSMainPhong", "PSMainPhong", kGBufferVariantPhong, 1u);
		if (!vs || !ps)
		{
			LOG::ERR("DeferredTask: gbuffer_screen_resolve.hlsl (Phong) compile failed");
			return;
		}
		Shader *shaders[] = { vs, ps };
		resolvePipelines[0] = device->CreateGraphicsPipeline();
		resolvePipelines[0]->Disable(Pipeline::State::Depth);
		resolvePipelines[0]->Construct(shaders, 2, {}, { Format::R16G16B16A16_SFLOAT });

		sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp, CompareOperation::Never, 0.0f, 16.0f);
		samplerShadowPoint = device->CreateSampler(
		    Filter::Nearest, /* mip */
		    Filter::Linear,  /* min */
		    Filter::Linear,  /* mag */
		    AddressMode::Clamp,
		    CompareOperation::Less);
		resolveDescriptorSets[0] = device->CreateDescriptorSet(resolvePipelines[0]);
		resolveDescriptorSets[0]->Set(11, sampler);
		resolveDescriptorSets[0]->Set(10, samplerShadowPoint);
		simpleFrameBuffer = device->CreateBuffer(BufferType::ConstantBuffer, kDeferredFrameBufferSize, MemoryType::Device);
		simpleFrameBuffer->SetDebugName("DeferredSimpleFrameBuffer");
		resolveDescriptorSets[0]->Set(7, simpleFrameBuffer);
		if (fallbackShadowDepthRT && fallbackShadowDepthRT->GetDepthAttachment())
		{
			Texture *fb = fallbackShadowDepthRT->GetDepthAttachment();
			for (uint32_t si = 0; si < 4u; ++si)
			{
				resolveDescriptorSets[0]->Set(3 + si, fb);
			}
		}

		{
			Ref<Shader> vsPbr = loadOrCompile(ShaderStage::Vertex, "gbuffer_screen_resolve_VS", "VSMain", kGBufferVariantPbr, 1u);
			Ref<Shader> psPbr = loadOrCompile(ShaderStage::Pixel, "gbuffer_screen_resolve_PSMainPBR", "PSMainPBR", kGBufferVariantPbr, 1u);
			if (vsPbr && psPbr)
			{
				Shader *shadersPbr[] = { vsPbr, psPbr };
				resolvePipelines[1] = device->CreateGraphicsPipeline();
				resolvePipelines[1]->Disable(Pipeline::State::Depth);
				resolvePipelines[1]->Construct(shadersPbr, 2, {}, { Format::R16G16B16A16_SFLOAT });
				resolveDescriptorSets[1] = device->CreateDescriptorSet(resolvePipelines[1]);
				resolveDescriptorSets[1]->Set(3, sampler);
				pbrFrameBuffer = device->CreateBuffer(BufferType::ConstantBuffer, kDeferredFrameBufferSize, MemoryType::Device);
				pbrFrameBuffer->SetDebugName("DeferredPbrFrameBuffer");
				resolveDescriptorSets[1]->Set(4, pbrFrameBuffer);
				resolveDescriptorSets[1]->Set(5, samplerShadowPoint);
				if (fallbackShadowDepthRT && fallbackShadowDepthRT->GetDepthAttachment())
				{
					Texture *fb = fallbackShadowDepthRT->GetDepthAttachment();
					for (uint32_t si = 0; si < 4u; ++si)
					{
						resolveDescriptorSets[1]->Set(9 + si, fb);
					}
				}
			}
			else
			{
				LOG::ERR("DeferredTask: PBR shader compile failed");
			}
		}

		{
			Ref<Shader> vsNpr = loadOrCompile(ShaderStage::Vertex, "gbuffer_screen_resolve_VS", "VSMain", kGBufferVariantNpr, 1u);
			Ref<Shader> psNpr = loadOrCompile(ShaderStage::Pixel, "gbuffer_screen_resolve_PSMainNPR", "PSMainNPR", kGBufferVariantNpr, 1u);
			if (vsNpr && psNpr)
			{
				Shader *shadersNpr[] = { vsNpr, psNpr };
				resolvePipelines[2] = device->CreateGraphicsPipeline();
				resolvePipelines[2]->Disable(Pipeline::State::Depth);
				resolvePipelines[2]->Construct(shadersNpr, 2, {}, { Format::R16G16B16A16_SFLOAT });
				resolveDescriptorSets[2] = device->CreateDescriptorSet(resolvePipelines[2]);
				resolveDescriptorSets[2]->Set(3, sampler);
				nprFrameBuffer = device->CreateBuffer(BufferType::ConstantBuffer, kDeferredFrameBufferSize, MemoryType::Device);
				nprFrameBuffer->SetDebugName("DeferredNprFrameBuffer");
				resolveDescriptorSets[2]->Set(4, nprFrameBuffer);
				resolveDescriptorSets[2]->Set(5, samplerShadowPoint);
				if (fallbackShadowDepthRT && fallbackShadowDepthRT->GetDepthAttachment())
				{
					Texture *fb = fallbackShadowDepthRT->GetDepthAttachment();
					for (uint32_t si = 0; si < 4u; ++si)
					{
						resolveDescriptorSets[2]->Set(9 + si, fb);
					}
				}
			}
			else
			{
				LOG::ERR("DeferredTask: NPR shader compile failed");
			}
		}

		resolvePipelines[3]     = resolvePipelines[0];
		resolveDescriptorSets[3] = resolveDescriptorSets[0];

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

	const uint32_t ri = (resolveLightingMode == 3u) ? 2u : ((resolveLightingMode == 2u) ? 1u : 0u);

	if (ri == 2u && resolvePipelines[2] && resolveDescriptorSets[2] && nprFrameBuffer)
	{
		Texture *depth = gbuffer->GetDepthAttachment();
		if (!depth)
		{
			return;
		}

		resolveDescriptorSets[2]->Set(0, alb);
		resolveDescriptorSets[2]->Set(1, nrm);
		resolveDescriptorSets[2]->Set(2, depth);

		DeferredResolveNPRConstants frame{};
		frame.invViewProjection = params.invViewProjection;
		frame.cameraWorld = params.cameraWorld;
		frame.view = params.view;
		if (params.lightCount == 0)
		{
			frame.lights[0].direction = Vector4{ 0.35f, 0.85f, 0.25f, 1.0f };
			frame.lights[0].radiance = Vector4{ 3.5f, 3.5f, 3.5f, 0.0f };
			frame.lights[0].position = Vector4{ 0.0f, 0.0f, 0.0f, 0.0f };
			frame.lightCount = 1;
		}
		else
		{
			frame.lightCount = params.lightCount;
			for (uint32_t i = 0; i < params.lightCount; i++)
			{
				frame.lights[i].direction  = params.lights[i].direction;
				frame.lights[i].radiance   = params.lights[i].radiance;
				frame.lights[i].position   = params.lights[i].position;
				frame.lights[i].spotParams = params.lights[i].spotParams;
			}
		}

		frame.shadowThreshold       = nprParams.shadowThreshold;
		frame.shadowSoftness        = nprParams.shadowSoftness;
		frame.litIntensity          = nprParams.litIntensity;
		frame.shadowColor[0]        = nprParams.shadowColor.x;
		frame.shadowColor[1]        = nprParams.shadowColor.y;
		frame.shadowColor[2]        = nprParams.shadowColor.z;
		frame.rimPower              = nprParams.rimPower;
		frame.rimIntensity          = nprParams.rimIntensity;
		frame.edgeDepthThreshold    = nprParams.edgeDepthThreshold;
		frame.edgeNormalThreshold   = nprParams.edgeNormalThreshold;
		frame.edgeIntensity         = nprParams.edgeIntensity;
		frame.exposure              = params.exposure;
		frame.gamma                 = params.gamma;
		frame.shadowViewProjection  = shadowResolveLightVP;
		frame.shadowCascadeCount    = std::min<uint32_t>(shadowResolveCascadeCount, SceneParameters::MaxShadowCascades);
		for (uint32_t c = 0; c < SceneParameters::MaxShadowCascades; c++)
		{
			frame.shadowCascadeViewProjection[c] = shadowResolveCascadeVP[c];
			frame.shadowCascadeSplits[c]         = shadowResolveCascadeSplits[c];
		}
		frame.shadowEnabled         = shadowResolveEnabled ? 1u : 0u;
		frame.shadowBias            = shadowResolveBias;
		frame.shadowStrength        = shadowResolveStrength;
		frame.shadowCasterLightIndex = shadowResolveCasterLightIndex;

		if (fallbackShadowDepthRT && fallbackShadowDepthRT->GetDepthAttachment())
		{
			Texture *fb = fallbackShadowDepthRT->GetDepthAttachment();
			for (uint32_t si = 0; si < 4u; ++si)
			{
				Texture *useTex = (shadowResolveEnabled && shadowResolveDepths[si]) ? shadowResolveDepths[si] : fb;
				resolveDescriptorSets[2]->Set(9 + si, useTex);
			}
		}

		Ref<Buffer> nprStaging = Graphics::GetCachedBuffer(BufferType::TransferSource, kDeferredFrameBufferSize);
		uint8_t nprPadded[kDeferredFrameBufferSize]{};
		std::memcpy(nprPadded, &frame, sizeof(frame));
		nprStaging->Fill(nprPadded, kDeferredFrameBufferSize, 0);
		commandBuffer->MemoryCopy(nprFrameBuffer, 0, nprStaging, 0, kDeferredFrameBufferSize);

		commandBuffer->SetPipeline(resolvePipelines[2]);
		commandBuffer->SetDescriptorSet(resolveDescriptorSets[2]);
		commandBuffer->DrawInstanced(3, 1, 0, 0);
		return;
	}

	if (ri == 1u && resolvePipelines[1] && resolveDescriptorSets[1] && pbrFrameBuffer)
	{
		Texture *depth = gbuffer->GetDepthAttachment();
		if (!depth)
		{
			return;
		}

		resolveDescriptorSets[1]->Set(0, alb);
		resolveDescriptorSets[1]->Set(1, nrm);
		resolveDescriptorSets[1]->Set(2, depth);

		DeferredResolvePBRFrameConstants frame{};
		frame.invViewProjection = params.invViewProjection;
		frame.cameraWorld = params.cameraWorld;
		frame.view = params.view;
		if (params.lightCount == 0)
		{
			frame.lights[0].direction = Vector4{ 0.35f, 0.85f, 0.25f, 1.0f };
			frame.lights[0].radiance = Vector4{ 4.0f, 4.0f, 4.0f, 0.0f };
			frame.lights[0].position = Vector4{ 0.0f, 0.0f, 0.0f, 0.0f };
			frame.lightCount = 1;
		}
		else
		{
			frame.lightCount = params.lightCount;
			for (uint32_t i = 0; i < params.lightCount; i++)
			{
				frame.lights[i].direction  = params.lights[i].direction;
				frame.lights[i].radiance   = params.lights[i].radiance;
				frame.lights[i].position   = params.lights[i].position;
				frame.lights[i].spotParams = params.lights[i].spotParams;
			}
		}

		frame.useIBL = 0;
		frame.maxSpecularLod = 0.0f;
		frame.exposure = params.exposure;
		frame.gamma = params.gamma;

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
		resolveDescriptorSets[1]->Set(6, irr);
		resolveDescriptorSets[1]->Set(7, pre);
		resolveDescriptorSets[1]->Set(8, brdf);
		{
			Texture *emTex = (gbuffer && gbuffer->GetColorAttachment(3)) ? gbuffer->GetColorAttachment(3) : nullptr;
			if (!emTex)
			{
				emTex = Graphics::Preset()->Textures.Black.Get();
			}
			if (emTex)
			{
				resolveDescriptorSets[1]->Set(13, emTex);
			}
		}

		frame.shadowViewProjection = shadowResolveLightVP;
		frame.shadowCascadeCount   = std::min<uint32_t>(shadowResolveCascadeCount, SceneParameters::MaxShadowCascades);
		for (uint32_t c = 0; c < SceneParameters::MaxShadowCascades; c++)
		{
			frame.shadowCascadeViewProjection[c] = shadowResolveCascadeVP[c];
			frame.shadowCascadeSplits[c]         = shadowResolveCascadeSplits[c];
		}
		frame.shadowEnabled      = shadowResolveEnabled ? 1u : 0u;
		frame.shadowBias         = shadowResolveBias;
		frame.shadowStrength     = shadowResolveStrength;
		frame.shadowCasterLightIndex = shadowResolveCasterLightIndex;

		if (fallbackShadowDepthRT && fallbackShadowDepthRT->GetDepthAttachment())
		{
			Texture *fb = fallbackShadowDepthRT->GetDepthAttachment();
			for (uint32_t si = 0; si < 4u; ++si)
			{
				Texture *useTex = (shadowResolveEnabled && shadowResolveDepths[si]) ? shadowResolveDepths[si] : fb;
				resolveDescriptorSets[1]->Set(9 + si, useTex);
			}
		}

		Ref<Buffer> pbrStaging = Graphics::GetCachedBuffer(BufferType::TransferSource, kDeferredFrameBufferSize);
		uint8_t pbrPadded[kDeferredFrameBufferSize]{};
		std::memcpy(pbrPadded, &frame, sizeof(frame));
		pbrStaging->Fill(pbrPadded, kDeferredFrameBufferSize, 0);
		commandBuffer->MemoryCopy(pbrFrameBuffer, 0, pbrStaging, 0, kDeferredFrameBufferSize);

		commandBuffer->SetPipeline(resolvePipelines[1]);
		commandBuffer->SetDescriptorSet(resolveDescriptorSets[1]);
		commandBuffer->DrawInstanced(3, 1, 0, 0);
		return;
	}

	if (!resolvePipelines[0] || !resolveDescriptorSets[0] || !simpleFrameBuffer)
	{
		return;
	}

	Texture *depth = gbuffer->GetDepthAttachment();
	if (!depth)
	{
		return;
	}

	resolveDescriptorSets[0]->Set(0, alb);
	resolveDescriptorSets[0]->Set(1, nrm);
	resolveDescriptorSets[0]->Set(2, depth);
	if (fallbackShadowDepthRT && fallbackShadowDepthRT->GetDepthAttachment())
	{
		Texture *fb = fallbackShadowDepthRT->GetDepthAttachment();
		for (uint32_t si = 0; si < 4u; ++si)
		{
			Texture *useTex = (shadowResolveEnabled && shadowResolveDepths[si]) ? shadowResolveDepths[si] : fb;
			resolveDescriptorSets[0]->Set(3 + si, useTex);
		}
	}

	DeferredResolveSimpleFrameConstants frame{};
	frame.invViewProjection = params.invViewProjection;
	frame.cameraWorld = params.cameraWorld;
	frame.view = params.view;
	if (params.lightCount == 0)
	{
		frame.lights[0].direction = Vector4{ 0.35f, 0.85f, 0.25f, 1.0f };
		frame.lights[0].radiance = Vector4{ 3.5f, 3.5f, 3.5f, 0.0f };
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

	frame.shadowViewProjection = shadowResolveLightVP;
	frame.shadowCascadeCount = std::min<uint32_t>(shadowResolveCascadeCount, SceneParameters::MaxShadowCascades);
	for (uint32_t c = 0; c < SceneParameters::MaxShadowCascades; c++)
	{
		frame.shadowCascadeViewProjection[c] = shadowResolveCascadeVP[c];
		frame.shadowCascadeSplits[c]         = shadowResolveCascadeSplits[c];
	}
	frame.shadowEnabled = shadowResolveEnabled ? 1u : 0u;
	frame.shadowBias = shadowResolveBias;
	frame.shadowStrength = shadowResolveStrength;
	frame.shadowCasterLightIndex = shadowResolveCasterLightIndex;

	Ref<Buffer> simpleStaging = Graphics::GetCachedBuffer(BufferType::TransferSource, kDeferredFrameBufferSize);
	uint8_t simplePadded[kDeferredFrameBufferSize]{};
	std::memcpy(simplePadded, &frame, sizeof(frame));
	simpleStaging->Fill(simplePadded, kDeferredFrameBufferSize, 0);
	commandBuffer->MemoryCopy(simpleFrameBuffer, 0, simpleStaging, 0, kDeferredFrameBufferSize);

	commandBuffer->SetPipeline(resolvePipelines[ri]);
	commandBuffer->SetDescriptorSet(resolveDescriptorSets[ri]);
	commandBuffer->DrawInstanced(3, 1, 0, 0);
}

void DeferredTask::SetShadowMapForResolve(
    Texture *d0,
    Texture *d1,
    Texture *d2,
    Texture *d3,
    const Matrix4 &lightViewProjection,
    const Matrix4 *cascadeViewProjection,
    const float *cascadeSplits,
    uint32_t cascadeCount,
    bool enabled,
    float bias,
    float strength,
    uint32_t casterLightIndex)
{
	shadowResolveDepths[0] = d0;
	shadowResolveDepths[1] = d1;
	shadowResolveDepths[2] = d2;
	shadowResolveDepths[3] = d3;
	shadowResolveLightVP   = lightViewProjection;
	shadowResolveCascadeCount = std::min<uint32_t>(cascadeCount, SceneParameters::MaxShadowCascades);
	for (uint32_t c = 0; c < SceneParameters::MaxShadowCascades; c++)
	{
		if (cascadeViewProjection && c < shadowResolveCascadeCount)
		{
			shadowResolveCascadeVP[c] = cascadeViewProjection[c];
		}
		else
		{
			shadowResolveCascadeVP[c] = lightViewProjection;
		}

		if (cascadeSplits && c < shadowResolveCascadeCount)
		{
			shadowResolveCascadeSplits[c] = cascadeSplits[c];
		}
		else
		{
			shadowResolveCascadeSplits[c] = FLT_MAX;
		}
	}
	shadowResolveEnabled   = enabled;
	shadowResolveBias      = bias;
	shadowResolveStrength  = strength;
	shadowResolveCasterLightIndex = casterLightIndex;
}

void DeferredTask::OnFrameGraphDebugGui()
{
	ImGui::TextUnformatted("Composite: fullscreen deferred resolve into current scene color.");
	if (!gbuffer)
	{
		ImGui::BulletText("G-buffer: not bound.");
		return;
	}
	FrameGraphDebugTextureThumbnail(gbuffer->GetColorAttachment(0), "G-buffer RT0 (albedo / material)");
	FrameGraphDebugTextureThumbnail(gbuffer->GetColorAttachment(1), "G-buffer RT1");
	FrameGraphDebugTextureThumbnail(gbuffer->GetColorAttachment(2), "G-buffer RT2 (object id)");
	FrameGraphDebugTextureThumbnail(gbuffer->GetDepthAttachment(), "G-buffer depth");
}

}
