#include "AtmosphereTask.h"

#include <imgui.h>

namespace Immortal
{

struct AtmospherePushConstants
{
	Matrix4 projection;

	Vector4 exposureGammaQualityPad;

	Vector4 sunDirIntensity;
};

AtmosphereTask::AtmosphereTask() :
    RenderTask{ "Atmosphere" }
{
}

AtmosphereTask::~AtmosphereTask()
{
}

void AtmosphereTask::SetSunDirection(const Vector3 &direction)
{
	sunDirection = direction;
}

void AtmosphereTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
		skybox = Mesh::CreateCube(asyncComputeThread, commandBuffer, 1.0f, true);

		auto device = Graphics::GetDevice();

		Ref<Shader> vertexShader{ Graphics::GetShaderByName("atmosphere_VS", ShaderStage::Vertex, "VSMain") };
		Ref<Shader> pixelShader{ Graphics::GetShaderByName("atmosphere_PS", ShaderStage::Pixel, "PSMain") };
		if (!vertexShader || !pixelShader)
		{
			LOG::ERR("AtmosphereTask: failed to load atmosphere_VS / atmosphere_PS shaders");
			return;
		}
		Shader *shaders[] = {
			vertexShader,
			pixelShader
		};
		Ref<GraphicsPipeline> graphicsPipeline = device->CreateGraphicsPipeline();
		graphicsPipeline->Enable(Pipeline::State::Depth);

		InputElementDescription inputElements = {
		    {
		        { Format::VECTOR3, "POSITION" },
		    }
		};
		inputElements.SetStride(sizeof(Mesh::SimpleVertex));
		graphicsPipeline->Construct(
		    shaders,
		    2,
		    inputElements,
		    {
		        Format::R8G8B8A8_UNORM,
		        Format::R32G32_UINT,
		    }
		);

		Graphics::StorePipeline("Atmosphere", graphicsPipeline);

		sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);
		dummyWhite = Graphics::Preset()->Textures.White;

		descriptorSet = device->CreateDescriptorSet(graphicsPipeline);
		descriptorSet->Set(0, dummyWhite);
		descriptorSet->Set(1, sampler);
	});
}

void AtmosphereTask::Execute(CommandBuffer *, const SceneParameters &)
{
}

void AtmosphereTask::Composite(CommandBuffer *commandBuffer, const SceneParameters &params)
{
	Ref<Pipeline> pipeline = Graphics::GetPipeline("Atmosphere");
	if (!enabled || !skybox || !pipeline || !descriptorSet)
	{
		return;
	}

	Vector3 sun = sunDirection;
	if (sun.Length() < 1e-6f)
	{
		sun = Vector3{ 0.35f, 0.85f, 0.25f };
	}
	sun = sun.Normalize();

	AtmospherePushConstants pc{};
	pc.projection = params.skyboxProjection;
	pc.exposureGammaQualityPad = Vector4{
	    1.8f,//params.exposure,
	    params.gamma,
	    (float)uint32_t(quality),
	    dayPhase,
	};
	pc.sunDirIntensity = Vector4{ sun, sunIntensity };

	commandBuffer->SetPipeline(pipeline);
	commandBuffer->SetDescriptorSet(descriptorSet);
	commandBuffer->PushConstants(ShaderStage::Vertex | ShaderStage::Pixel, &pc, sizeof(pc), 0);

	auto &nodes = skybox->NodeList();
	Buffer *vertexBuffers[] = { nodes[0].Vertex };
	commandBuffer->SetVertexBuffers(0, 1, vertexBuffers, sizeof(Mesh::SimpleVertex));
	commandBuffer->SetIndexBuffer(nodes[0].Index, Format::UINT32);
	commandBuffer->DrawIndexedInstance(nodes[0].Index->GetSize() / sizeof(uint32_t), 1, 0, 0, 0);
}

void AtmosphereTask::OnFrameGraphDebugGui()
{
	ImGui::Text("Enabled: %s", enabled ? "yes" : "no");
	ImGui::BulletText("Procedural sky drawn in Composite; no off-screen debug texture.");
}

}
