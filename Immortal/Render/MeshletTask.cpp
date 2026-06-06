#include "MeshletTask.h"
#include "Vision/Image.h"

namespace Immortal
{

namespace
{

struct MeshletPushConstants
{
	Matrix4 MVP;
	Matrix4 Model;
	uint32_t objectId;
	uint32_t submeshIndex;
	float roughness;
	float metallic;
	uint32_t pad0;
};

}

MeshletTask::MeshletTask() :
    RenderTask{ "Meshlet" }
{
}

MeshletTask::~MeshletTask()
{
}

struct UBO
{
	Matrix4 viewProjection;
	Matrix4 model;
};

void MeshletTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t value, CommandBuffer *commandBuffer) {

		auto device = Graphics::GetDevice();
		auto source = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "meshlet.hlsl");
		URef<Shader> meshShader = device->CreateShader("Meshlet", ShaderStage::Mesh, source, "MSMain");
		URef<Shader> pixelForward = device->CreateShader("MeshletPixel", ShaderStage::Pixel, source, "PSMain");
		URef<Shader> pixelGBuffer = device->CreateShader("MeshletPixelGBuffer", ShaderStage::Pixel, source, "PSMainGBuffer");
		Shader *forwardShaders[] = {
			meshShader,
			pixelForward
		};
		Shader *deferredShaders[] = {
			meshShader,
			pixelGBuffer
		};

		pipelineForward = device->CreateGraphicsPipeline();
		pipelineForward->Enable(Pipeline::State::Depth);
		pipelineForward->Construct(forwardShaders, 2, {},
			{
			    Format::R8G8B8A8_UNORM,
			    Format::R32G32_UINT,
			    Format::Depth24Stencil8
			}
		);

		pipelineDeferred = device->CreateGraphicsPipeline();
		pipelineDeferred->Enable(Pipeline::State::Depth);
		pipelineDeferred->Construct(deferredShaders, 2, {},
			{
			    Format::R8G8B8A8_UNORM,
			    Format::R8G8B8A8_UNORM,
			    Format::R32G32_UINT,
			    Format::Depth24Stencil8
			}
		);

		stagingBuffer = Graphics::GetCachedBuffer(BufferType::TransferSource, sizeof(SceneConstantBuffer));
		constantBuffer = device->CreateBuffer(BufferType::ConstantBuffer, sizeof(SceneConstantBuffer), MemoryType::Device);
		constantBuffer->SetDebugName("SceneConstantBuffer");
		sampler = device->CreateSampler(Filter::Linear, AddressMode::Wrap);
	});
}

void MeshletTask::Execute(CommandBuffer *commandBuffer, const SceneParameters &params)
{
	SceneConstantBuffer constantBufferData = {
		.World         = glm::identity<glm::mat4x4>(),
		.WorldView     = params.view,
		.WorldViewProj = params.viewProjection,
		.DrawMeshlets  = 1
	};

	stagingBuffer->Fill(&constantBufferData, sizeof(constantBufferData), 0);
	commandBuffer->MemoryCopy(constantBuffer, 0, stagingBuffer, 0, sizeof(SceneConstantBuffer));
}

void MeshletTask::Composite(CommandBuffer *commandBuffer, const SceneParameters &params)
{

}

void MeshletTask::DrawMesh(CommandBuffer *commandBuffer, const SceneParameters &params, uint32_t objectId, const TransformComponent &transform, MeshComponent &meshComponent, const MaterialComponent &materialComponent)
{
	Ref<GraphicsPipeline> activePipeline = (renderPath == MeshletRenderPath::Deferred) ? pipelineDeferred : pipelineForward;
	if (!activePipeline)
	{
		return;
	}

	const Ref<Mesh> &mesh = meshComponent.Mesh;
	if (!mesh)
	{
		return;
	}

	Matrix4 baseModel = transform.Transform();
	meshComponent.EnsureSubmeshLocalCount(mesh->NodeList().size());

	MeshletPushConstants pc{};
	pc.objectId = objectId;
	pc.submeshIndex = 0;
	pc.pad0 = 0;

	commandBuffer->SetPipeline(activePipeline);

	auto &nodes = mesh->NodeList();
	for (size_t ni = 0; ni < nodes.size(); ni++)
	{
		auto &node = nodes[ni];
		if (ni >= materialComponent.References.size())
		{
			continue;
		}

		Matrix4 local = ni < meshComponent.SubmeshLocalTransform.size() ? meshComponent.SubmeshLocalTransform[ni] : Matrix4(1.0f);
		Matrix4 model = baseModel * local;
		pc.MVP = params.viewProjection * model;
		pc.Model = model;
		pc.submeshIndex = (uint32_t)ni;

		auto &descriptorSet = node.descriptorSet;
		if (!descriptorSet)
		{
			auto device = Graphics::GetDevice();
			descriptorSet = device->CreateDescriptorSet(activePipeline);

			descriptorSet->Set(0, node.Vertex);
			descriptorSet->Set(1, node.Meshlets);
			descriptorSet->Set(2, node.UniqueVertexIndices);
			descriptorSet->Set(3, node.PrimitiveIndices);
			descriptorSet->Set(5, sampler);
		}
		const auto &mat = materialComponent.References[ni];
		pc.roughness = mat.Roughness;
		pc.metallic = mat.Metallic;
		commandBuffer->PushConstants(ShaderStage::Mesh | ShaderStage::Pixel, &pc, sizeof(pc), 0);
		descriptorSet->Set(4, mat.Textures.Albedo);
		commandBuffer->SetDescriptorSet(descriptorSet);
		commandBuffer->DispatchMeshTasks(node.MeshletSubsetCount, 1, 1);
	}
}

}
