#include "MeshletShadowTask.h"

#include "Graphics/AsyncCompute.h"

#include <cstring>

namespace Immortal
{

struct MeshletPushConstants
{
	Matrix4 MVP;
	Matrix4 Model;
};

MeshletShadowTask::MeshletShadowTask() :
    RenderTask{ "MeshletShadow", Flags::MeshRendering },
	ICLASS
{

}

MeshletShadowTask::~MeshletShadowTask() = default;

void MeshletShadowTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
		auto device = Graphics::GetDevice();

		const std::string name = "meshlet_shadow_depth";
		URef<Shader> meshShader  = Graphics::GetShaderByName(name + "_MS", ShaderStage::Mesh, "MSMain");
		URef<Shader> pixelShader = Graphics::GetShaderByName(name + "_PS", ShaderStage::Pixel, "PSMain");

		if (!meshShader || !pixelShader)
		{
			LOG_ERROR("Failed to create shader for {}", name);
			return;
		}

		Shader *shadowShaders[] = {meshShader, pixelShader};
		pipeline = device->CreateGraphicsPipeline();
		pipeline->Enable(Pipeline::State::Depth | Pipeline::State::ShadowPass);
		pipeline->Construct(shadowShaders, 2, {}, {Format::RGBA8, Format::Depth24Stencil8});
	});
}

void MeshletShadowTask::Execute(CommandBuffer *, const SceneParameters &)
{
}

void MeshletShadowTask::Composite(CommandBuffer *, const SceneParameters &)
{
}

void MeshletShadowTask::DrawMesh(CommandBuffer *commandBuffer, const SceneParameters &params, uint32_t objectId, const TransformComponent &transform, MeshComponent &meshComponent, const MaterialComponent &materialComponent, const RenderPass *pass)
{
	if (!pipeline || !params.shadowEnabled)
	{
		return;
	}

	const Ref<Mesh> &mesh = meshComponent.Mesh;
	if (!mesh || mesh->IsSkinned())
	{
		return;
	}

	const uint32_t cascadeSliceOnly = pass->shadowCascadeIndex;

	Matrix4 baseModel = transform.Transform();
	meshComponent.EnsureSubmeshLocalCount(mesh->NodeList().size());

	MeshletPushConstants pc{};
	commandBuffer->SetPipeline(pipeline);

	auto &nodes = mesh->NodeList();
	for (size_t i = 0; i < nodes.size(); i++)
	{
		auto &node = nodes[i];
		if (i >= materialComponent.References.size())
		{
			continue;
		}

		Matrix4 local   = i < meshComponent.SubmeshLocalTransform.size() ? meshComponent.SubmeshLocalTransform[i] : Matrix4(1.0f);
		Matrix4 model   = baseModel * local;
		pc.Model        = model;

		auto &descriptorSet = node.shadowDescriptorSet;
		if (!descriptorSet)
		{
			auto device = Graphics::GetDevice();
			descriptorSet = device->CreateDescriptorSet(pipeline);

			descriptorSet->Set(0, node.Vertex);
			descriptorSet->Set(1, node.Meshlets);
			descriptorSet->Set(2, node.UniqueVertexIndices);
			descriptorSet->Set(3, node.PrimitiveIndices);
		}

		const uint32_t ci = (cascadeSliceOnly >= SceneParameters::MaxShadowCascades) ? 0u : cascadeSliceOnly;
		pc.MVP = params.shadowCascadeViewProjection[ci] * model;
		commandBuffer->PushConstants(ShaderStage::Mesh | ShaderStage::Pixel, &pc, sizeof(pc), 0);
		commandBuffer->SetDescriptorSet(descriptorSet);
		commandBuffer->DispatchMeshTasks(node.MeshletSubsetCount, 1, 1);
	}
}

void MeshletShadowTask::OnFrameGraphDebugGui()
{
	ImGui::TextUnformatted("Static meshlet shadow depth (meshlet_shadow_depth.hlsl).");
}

}
