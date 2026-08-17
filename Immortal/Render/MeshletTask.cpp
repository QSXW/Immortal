#include "MeshletTask.h"
#include "Render/IBLTask.h"
#include "Render/FrameGraphDebugUi.h"
#include "Render/RenderPass.h"
#include "Scene/GameScene.h"

#include <imgui.h>
#include <algorithm>
#include <cstring>

namespace Immortal
{

struct MeshletPushConstants
{
	Matrix4  MVP;
	Matrix4  Model;
	uint32_t objectId;
	uint32_t submeshIndex;
	float    roughness;
	float    metallic;
	Vector4  albedoColor;
	Vector4  emissive;
};

struct SceneGPU
{
	Vector4  lightDirection;
	Vector4  lightRadiance;
	Matrix4  shadowViewProjection;
	uint32_t shadowEnabled;
	uint32_t shadowEnablePcf;
	float    shadowBias;
	float    shadowStrength;
	uint32_t shadowVisualizeRgbMode;
	float    shadowVisualizePad[3];
	uint32_t iblUse;
	float    iblMaxSpecularLod;
	uint32_t iblPad0;
	uint32_t iblPad1;
};

struct ShadowCascadeGPU
{
	Matrix4 cascadeVP[4];
	Vector4 splits;
	uint32_t cascadeCount;
	uint32_t _pad[3];
	Matrix4 view;
	Vector4 cameraWorldPos;
};

static constexpr size_t kCascadeCbBytes = 384;

static int s_meshletShadowRgbVizMode = 0;

MeshletTask::MeshletTask() :
    RenderTask{ "Meshlet", Flags::MeshRendering }
{
}

MeshletTask::~MeshletTask() = default;

void MeshletTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
		auto device = Graphics::GetDevice();

		auto loadShader = [](ShaderStage stage, const std::string &name, const char *entryPoint) -> URef<Shader> {
			return Graphics::GetShaderByName(name, stage, entryPoint);
		};

		URef<Shader> meshShader = loadShader(ShaderStage::Mesh,  "meshlet_MS",            "MSMain"       );
		URef<Shader> psUnlit    = loadShader(ShaderStage::Pixel, "meshlet_PS",            "PSMain"       );
		URef<Shader> psPhong    = loadShader(ShaderStage::Pixel, "meshlet_PSMainPhong",   "PSMainPhong"  );
		URef<Shader> psPbr      = loadShader(ShaderStage::Pixel, "meshlet_PSMainPBR",     "PSMainPBR"    );
		URef<Shader> psNpr      = loadShader(ShaderStage::Pixel, "meshlet_PSMainNPR",     "PSMainNPR"    );
		URef<Shader> psGbuf     = loadShader(ShaderStage::Pixel, "meshlet_PSMainGBuffer", "PSMainGBuffer");

		if (!meshShader || !psUnlit || !psPhong || !psPbr || !psNpr || !psGbuf)
		{
			LOG::ERR("MeshletTask: failed to load meshlet shaders.");
			return;
		}

		Shader *shUnlit[] = { meshShader, psUnlit };
		pipelines[(uint32_t)MeshletPipelineSlot::Unlit] = device->CreateGraphicsPipeline();
		pipelines[(uint32_t)MeshletPipelineSlot::Unlit]->Enable(Pipeline::State::Depth);
		pipelines[(uint32_t)MeshletPipelineSlot::Unlit]->Construct(shUnlit, 2, {},
		    { Format::R16G16B16A16_SFLOAT, Format::R32G32_UINT, Format::Depth24Stencil8 });

		Shader *shPhong[] = { meshShader, psPhong };
		pipelines[(uint32_t)MeshletPipelineSlot::Phong] = device->CreateGraphicsPipeline();
		pipelines[(uint32_t)MeshletPipelineSlot::Phong]->Enable(Pipeline::State::Depth);
		pipelines[(uint32_t)MeshletPipelineSlot::Phong]->Construct(shPhong, 2, {},
		    { Format::R16G16B16A16_SFLOAT, Format::R32G32_UINT, Format::Depth24Stencil8 });

		Shader *shPbr[] = { meshShader, psPbr };
		pipelines[(uint32_t)MeshletPipelineSlot::PBR] = device->CreateGraphicsPipeline();
		pipelines[(uint32_t)MeshletPipelineSlot::PBR]->Enable(Pipeline::State::Depth);
		pipelines[(uint32_t)MeshletPipelineSlot::PBR]->Construct(shPbr, 2, {},
		    { Format::R16G16B16A16_SFLOAT, Format::R32G32_UINT, Format::Depth24Stencil8 });

		Shader *shNpr[] = { meshShader, psNpr };
		pipelines[(uint32_t)MeshletPipelineSlot::NPR] = device->CreateGraphicsPipeline();
		pipelines[(uint32_t)MeshletPipelineSlot::NPR]->Enable(Pipeline::State::Depth);
		pipelines[(uint32_t)MeshletPipelineSlot::NPR]->Construct(shNpr, 2, {},
		    { Format::R16G16B16A16_SFLOAT, Format::R32G32_UINT, Format::Depth24Stencil8 });

		Shader *shGbuf[] = { meshShader, psGbuf };
		pipelines[(uint32_t)MeshletPipelineSlot::GBuffer] = device->CreateGraphicsPipeline();
		pipelines[(uint32_t)MeshletPipelineSlot::GBuffer]->Enable(Pipeline::State::Depth);
		pipelines[(uint32_t)MeshletPipelineSlot::GBuffer]->Construct(shGbuf, 2, {},
		    { Format::R16G16B16A16_SFLOAT, Format::R16G16B16A16_SFLOAT, Format::R32G32_UINT, Format::R8G8B8A8_UNORM, Format::Depth24Stencil8 });

		sceneBuffer   = device->CreateBuffer(BufferType::ConstantBuffer, sizeof(SceneGPU), MemoryType::Device);
		cascadeBuffer = device->CreateBuffer(BufferType::ConstantBuffer, kCascadeCbBytes, MemoryType::Device);
		sceneBuffer->SetDebugName("MeshletSceneBuffer");
		cascadeBuffer->SetDebugName("MeshletCascadeBuffer");

		sceneStaging   = device->CreateBuffer(BufferType::TransferSource, sizeof(SceneGPU), MemoryType::Host);
		cascadeStaging = device->CreateBuffer(BufferType::TransferSource, kCascadeCbBytes, MemoryType::Host);

		shadowSamplerClamp   = device->CreateSampler(Filter::Nearest, Filter::Linear, Filter::Linear, AddressMode::Clamp, CompareOperation::Never);
		shadowPassRgbSampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);
		iblEnvSamplerClamp   = device->CreateSampler(Filter::Linear, AddressMode::Clamp, CompareOperation::Never, 0.0f, 16.0f);

		ClearValue shadowDepthClear[1] = { { .depthStencil = { 1.0f, 0 } } };
		fallbackShadowDepthRT = device->CreateRenderTarget(1, 1, nullptr, 0, Format::Depth24Stencil8, shadowDepthClear);

		Format shadowRgbFmt = Format::R16G16B16A16_SFLOAT;
		ClearValue shadowRgbClear{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } };
		fallbackShadowPassRgbRT = device->CreateRenderTarget(1, 1, &shadowRgbFmt, 1, Format::None, &shadowRgbClear, 1);

		stagingBuffer  = Graphics::GetCachedBuffer(BufferType::TransferSource, sizeof(SceneConstantBuffer));
		constantBuffer = device->CreateBuffer(BufferType::ConstantBuffer, sizeof(SceneConstantBuffer), MemoryType::Device);
		constantBuffer->SetDebugName("MeshletSceneConstantBuffer");
		sampler = device->CreateSampler(Filter::Linear, AddressMode::Wrap);
	});
}

void MeshletTask::SetShadowMaps(Texture *d0, Texture *d1, Texture *d2, Texture *d3, Texture *shadowDepthRgbViz)
{
	shadowDepths[0] = d0;
	shadowDepths[1] = d1;
	shadowDepths[2] = d2;
	shadowDepths[3] = d3;
	shadowDepthRgbViz = shadowDepthRgbViz;
}

void MeshletTask::SetFrameEnvironmentMaps(Texture *irradiance, Texture *prefilter, Texture *brdfLut, uint32_t useIBL, float maxSpecularLod)
{
	frameIrradianceCube   = irradiance;
	framePrefilterCube    = prefilter;
	frameBrdfLut          = brdfLut;
	frameUseIBL           = useIBL;
	frameMaxSpecularLod   = maxSpecularLod;
}

void MeshletTask::SetIBLTask(IBLTask *ibl)
{
	linkedIbl = ibl;
}

void MeshletTask::Execute(CommandBuffer *commandBuffer, const SceneParameters &params)
{
	if (linkedIbl)
	{
		float maxLod = 0.0f;
		if (linkedIbl->GetPrefilterRadianceMap())
		{
			uint32_t mips = (uint32_t)std::max(1, (int)linkedIbl->GetPrefilterRadianceMap()->GetMipLevels());
			maxLod = (float)std::max(0, (int)mips - 1);
		}
		SetFrameEnvironmentMaps(
		    linkedIbl->GetIrradianceMap().Get(),
		    linkedIbl->GetPrefilterRadianceMap().Get(),
		    linkedIbl->GetBRDFLUT().Get(),
		    linkedIbl->IsIBLActive() ? 1u : 0u,
		    maxLod);
	}
	else
	{
		SetFrameEnvironmentMaps(nullptr, nullptr, nullptr, 0u, 0.0f);
	}

	SceneConstantBuffer constantBufferData = {
		.World         = glm::identity<glm::mat4x4>(),
		.WorldView     = params.view,
		.WorldViewProj = params.viewProjection,
		.DrawMeshlets  = 1
	};

	stagingBuffer->Fill(&constantBufferData, sizeof(constantBufferData), 0);
	commandBuffer->MemoryCopy(constantBuffer, 0, stagingBuffer, 0, sizeof(SceneConstantBuffer));

	Vector4 lightDir{ 0.35f, 0.85f, 0.25f, 1.0f };
	Vector4 lightRad{ 3.5f, 3.5f, 3.5f, 0.0f };
	if (params.lightCount > 0)
	{
		uint32_t lightIndex = (params.shadowCasterLightIndex < params.lightCount) ? params.shadowCasterLightIndex : 0u;
		lightDir = params.lights[lightIndex].direction;
		lightRad = params.lights[lightIndex].radiance;
	}

	SceneGPU scene{};
	scene.lightDirection       = lightDir;
	scene.lightRadiance        = lightRad;
	scene.shadowViewProjection = params.shadowViewProjection;
	int viz = s_meshletShadowRgbVizMode;
	viz     = std::max(0, std::min(3, viz));
	scene.shadowVisualizeRgbMode = (uint32_t)viz;
	scene.shadowVisualizePad[0]  = 0.0f;
	scene.shadowVisualizePad[1]  = 0.0f;
	scene.shadowVisualizePad[2]  = 0.0f;
	scene.shadowEnabled          = params.shadowEnabled;
	scene.shadowBias             = params.shadowBias;
	scene.shadowStrength         = params.shadowStrength;
	scene.shadowEnablePcf        = params.shadowEnablePcf;
	scene.iblUse                 = frameUseIBL;
	scene.iblMaxSpecularLod      = frameMaxSpecularLod;
	scene.iblPad0                = 0;
	scene.iblPad1                = 0;

	sceneStaging->Fill(&scene, sizeof(scene), 0);
	commandBuffer->MemoryCopy(sceneBuffer, 0, sceneStaging, 0, sizeof(SceneGPU));

	if (cascadeBuffer)
	{
		ShadowCascadeGPU casc{};
		casc.view = params.view;
		casc.cameraWorldPos = params.cameraWorld;
		casc.cascadeCount = std::min<uint32_t>(params.shadowCascadeCount, SceneParameters::MaxShadowCascades);
		for (uint32_t c = 0; c < SceneParameters::MaxShadowCascades; c++)
		{
			casc.cascadeVP[c] = params.shadowCascadeViewProjection[c];
		}
		casc.splits = Vector4{
		    params.shadowCascadeSplits[0],
		    params.shadowCascadeSplits[1],
		    params.shadowCascadeSplits[2],
		    params.shadowCascadeSplits[3],
		};

		uint8_t cascPadded[kCascadeCbBytes]{};
		std::memcpy(cascPadded, &casc, sizeof(casc));
		cascadeStaging->Fill(cascPadded, kCascadeCbBytes, 0);
		commandBuffer->MemoryCopy(cascadeBuffer, 0, cascadeStaging, 0, kCascadeCbBytes);
	}
}

void MeshletTask::Composite(CommandBuffer *, const SceneParameters &)
{
}

void MeshletTask::OnFrameGraphDebugGui()
{
	ImGui::TextUnformatted("Meshlets: static (meshlet.hlsl) + skinned (meshlet_skinned.hlsl, bone buffer t6).");
	ImGui::BulletText("Skinned shadow uses meshlet_skinned_shadow_depth.hlsl in shadow passes.");
	const char *vizItems[] = {
		"Off (lit shading)",
		"Shadow factor as RGB (grayscale)",
		"Light-space UVZ (saturate, debug)",
		"Shadow pass RT0 RGB (depth viz PS)"
	};
	ImGui::Combo("Meshlet shadow RGB viz", &s_meshletShadowRgbVizMode, vizItems, 4);
	if (shadowDepthRgbViz)
	{
		ImGui::Separator();
		FrameGraphDebugTextureThumbnail(shadowDepthRgbViz, "Shadow pass depth as RGB");
	}
}

void MeshletTask::DrawMesh(
    CommandBuffer *commandBuffer,
    const SceneParameters &params,
    uint32_t objectId,
    const TransformComponent &transform,
    MeshComponent &meshComponent,
    const MaterialComponent &materialComponent,
    const RenderPass *pass)
{
	const Ref<Mesh> &mesh = meshComponent.Mesh;

	const uint32_t cascadeSliceOnly = pass->shadowCascadeIndex;
	Matrix4 baseModel = transform.Transform();
	meshComponent.EnsureSubmeshLocalCount(mesh->NodeList().size());

	if (mesh->IsSkinned())
	{
		return;
	}

	const uint32_t pipelineIndex =
	    pass->isGBufferPass ? (uint32_t) MeshletPipelineSlot::GBuffer : pass->lightingMode;
	auto          &pipeline = pipelines[pipelineIndex];
	MeshletPushConstants pc{};
	commandBuffer->SetPipeline(pipeline);

	auto &nodes = mesh->NodeList();
	Ref<Buffer> boneBuffer = mesh->GetTransforms();
	for (size_t i = 0; i < nodes.size(); i++)
	{
		auto &node = nodes[i];
		if (i >= materialComponent.References.size())
		{
			continue;
		}

		const auto &material = materialComponent.References[i];

		Matrix4 local = i < meshComponent.SubmeshLocalTransform.size() ? meshComponent.SubmeshLocalTransform[i] : Matrix4(1.0f);
		Matrix4 model = baseModel * local;
		pc.Model        = model;
		pc.submeshIndex = (uint32_t) i;
		pc.roughness    = material.Roughness;
		pc.metallic     = material.Metallic;
		pc.albedoColor  = material.AlbedoColor;
		pc.emissive     = material.Emissive;

		auto &descriptorSet = node.descriptorSet[pass->lightingMode];
		if (!descriptorSet)
		{
			auto *device = Graphics::GetDevice();
			descriptorSet = device->CreateDescriptorSet(pipeline);
			descriptorSet->Set(0, node.Vertex             );
			descriptorSet->Set(1, node.Meshlets           );
			descriptorSet->Set(2, node.UniqueVertexIndices);
			descriptorSet->Set(3, node.PrimitiveIndices   );

			descriptorSet->Set(4, sampler);

			auto &texture = material.Textures;
			descriptorSet->Set(5, texture.Albedo);

			if (pass->lightingMode == (uint32_t)SceneLightingModel::Phong || 
				pass->lightingMode == (uint32_t)SceneLightingModel::PBR   ||
			    pass->lightingMode == (uint32_t)SceneLightingModel::NPR)
			{
				descriptorSet->Set(6, sceneBuffer.Get());
				descriptorSet->Set(7, cascadeBuffer.Get());
				descriptorSet->Set(8, shadowSamplerClamp);
			}
		}
		
		if (pass->lightingMode == (uint32_t)SceneLightingModel::Phong || 
			pass->lightingMode == (uint32_t)SceneLightingModel::PBR   ||
		    pass->lightingMode == (uint32_t)SceneLightingModel::NPR)
		{
			{
				Texture *fallbackDepth = fallbackShadowDepthRT ? fallbackShadowDepthRT->GetDepthAttachment() : nullptr;
				for (size_t si = 0; si < 4; si++)
				{
					Texture *td = shadowDepths[si] ? shadowDepths[si] : fallbackDepth;
					if (td)
					{
						descriptorSet->Set(9 + si, td);
					}
				}
			}

			if (!pass->isGBufferPass && SceneLightingModel(pass->lightingMode) == SceneLightingModel::PBR)
			{
				descriptorSet->Set(14, iblEnvSamplerClamp);
				if (frameIrradianceCube)
				{
					descriptorSet->Set(15, frameIrradianceCube);
				}
				if (framePrefilterCube)
				{
					descriptorSet->Set(16, framePrefilterCube);
				}
				if (frameBrdfLut)
				{
					descriptorSet->Set(17, frameBrdfLut);
				}
			}
		}
		/* MSMain clip position uses Cam.MVP; shadow depth uses MeshletShadowTask — this path is GBuffer / forward scene draw. */
		if (pass->isShadowDepthPass)
		{
			uint32_t ci = cascadeSliceOnly;
			if (ci >= SceneParameters::MaxShadowCascades)
			{
				ci = 0u;
			}
			pc.MVP = params.shadowCascadeViewProjection[ci] * model;
		}
		else
		{
			pc.MVP = params.viewProjection * model;
		}
		pc.objectId = objectId;

		commandBuffer->PushConstants(ShaderStage::Mesh | ShaderStage::Pixel, &pc, sizeof(pc), 0);
		commandBuffer->SetDescriptorSet(descriptorSet);
		commandBuffer->DispatchMeshTasks(node.MeshletSubsetCount, 1, 1);
	}
}

}
