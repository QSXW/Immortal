#include "MeshletTask.h"
#include "Vision/Image.h"

namespace Immortal
{

MeshletTask::MeshletTask() :
    RenderTask{"Meshlet"}
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
		//auto source = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "meshshader.hlsl"/*"meshlet.hlsl"*/);
		auto source = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "meshlet.hlsl");
		URef<Shader> meshShader  = device->CreateShader("Meshlet",      ShaderStage::Mesh,  source, "MSMain");
		URef<Shader> pixelShader = device->CreateShader("MeshletPixel", ShaderStage::Pixel, source, "PSMain");
		Shader *shaders[] = {
		    meshShader, pixelShader
		};
		pipeline = device->CreateGraphicsPipeline();
		pipeline->Enable(Pipeline::State::Depth);
		pipeline->Construct(shaders, 2, {},
			{
				Format::R8G8B8A8_UNORM,
				Format::R32_UINT,
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

	//UBO ubo = {
	//    .viewProjection = params.viewProjection,
	//    .model = glm::identity<glm::mat4x4>(),
	//};

	//stagingBuffer->Fill(&ubo, sizeof(ubo), 0);
	//commandBuffer->MemoryCopy(constantBuffer, 0, stagingBuffer, 0, sizeof(UBO));

	stagingBuffer->Fill(&constantBufferData, sizeof(constantBufferData), 0);
	commandBuffer->MemoryCopy(constantBuffer, 0, stagingBuffer, 0, sizeof(SceneConstantBuffer));
}

void MeshletTask::Composite(CommandBuffer *commandBuffer, const SceneParameters &params)
{

}

void MeshletTask::DrawMesh(CommandBuffer *commandBuffer, const SceneParameters &params, uint32_t objectId, const TransformComponent &transform, const Ref<Mesh> &mesh, const MaterialComponent &materialComponent)
{
 	MeshInfo meshInfo{
		.Model         = transform,
	    .IndexBytes    = 4,
	    .MeshletOffset = 0,
	};

	UBO ubo = {
	    .viewProjection = params.viewProjection,
	    .model = glm::identity<glm::mat4x4>(),
	};

	//Matrix4 mvp = params.viewProjection * transform.Transform();
	//stagingBuffer->Fill(&mvp, sizeof(mvp), 0);
	//commandBuffer->MemoryCopy(constantBuffer, 0, stagingBuffer, 0, sizeof(mvp));

	commandBuffer->SetPipeline(pipeline);
	//commandBuffer->PushConstants(ShaderStage::Mesh, &meshInfo, sizeof(meshInfo), 0);

	Matrix4 mvp = params.viewProjection * transform.Transform();
	commandBuffer->PushConstants(ShaderStage::Mesh|ShaderStage::Pixel, &mvp, sizeof(mvp), 0);
	commandBuffer->PushConstants(ShaderStage::Mesh|ShaderStage::Pixel, &objectId, sizeof(objectId), sizeof(mvp));

	for (auto &node : mesh->NodeList())
	{
		auto &descriptorSet = node.descriptorSet;
		if (!descriptorSet)
		{
			auto device = Graphics::GetDevice();
			descriptorSet = device->CreateDescriptorSet(pipeline);

			//descriptorSet->Set(0, constantBuffer          );
			//descriptorSet->Set(1, node.Vertex             );
			//descriptorSet->Set(2, node.Meshlets           );
			//descriptorSet->Set(3, node.UniqueVertexIndices);
			//descriptorSet->Set(4, node.PrimitiveIndices   );

			descriptorSet->Set(0, node.Vertex);
			descriptorSet->Set(1, node.Meshlets);
			descriptorSet->Set(2, node.UniqueVertexIndices);
			descriptorSet->Set(3, node.PrimitiveIndices);
			descriptorSet->Set(5, sampler);
		}
		descriptorSet->Set(4, materialComponent.References[0].Textures.Albedo);
		commandBuffer->SetDescriptorSet(descriptorSet );
		// commandBuffer->PushConstants(ShaderStage::Mesh, &meshInfo.MeshletOffset, sizeof(meshInfo.MeshletOffset), sizeof(meshInfo.Model) + sizeof(meshInfo.IndexBytes));
		commandBuffer->DispatchMeshTasks(node.MeshletSubsetCount, 1, 1);
	}
}

}
