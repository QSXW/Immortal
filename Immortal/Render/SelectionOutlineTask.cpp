#include "SelectionOutlineTask.h"

namespace Immortal
{

namespace
{

struct OutlinePushConstants
{
	uint32_t selectedId;
	uint32_t selectedSubMesh;
	float    thickness;
	float    _pad;
	float    color[4];
};

}

SelectionOutlineTask::SelectionOutlineTask()
{
}

SelectionOutlineTask::~SelectionOutlineTask()
{
}

void SelectionOutlineTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
		auto device = Graphics::GetDevice();

		Ref<Shader> vs = Graphics::GetShaderByName("selection_outline_VS", ShaderStage::Vertex, "VSMain");
		Ref<Shader> ps = Graphics::GetShaderByName("selection_outline_PS", ShaderStage::Pixel, "PSMain");
		if (!vs || !ps)
		{
			LOG::ERR("SelectionOutlineTask: shader compile failed");
			return;
		}

		Shader *shaders[] = { vs, ps };
		pipeline = device->CreateGraphicsPipeline();
		pipeline->Disable(Pipeline::State::Depth);
		pipeline->Construct(shaders, 2, {}, { Format::R8G8B8A8_UNORM });

		sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);
		descriptorSet = device->CreateDescriptorSet(pipeline);
		descriptorSet->Set(2, sampler);

		built = true;
	});
}

void SelectionOutlineTask::Execute(CommandBuffer *commandBuffer, Texture *sceneColor, Texture *objectIdTexture)
{
	if (!built || !pipeline || !descriptorSet || !outputTarget || !sceneColor || !objectIdTexture)
	{
		return;
	}

	descriptorSet->Set(0, sceneColor);
	descriptorSet->Set(1, objectIdTexture);

	OutlinePushConstants pc{};
	pc.selectedId      = selectedObjectId;
	pc.selectedSubMesh = selectedSubMesh;
	pc.thickness       = thickness;
	pc.color[0]        = outlineColor.x;
	pc.color[1]        = outlineColor.y;
	pc.color[2]        = outlineColor.z;
	pc.color[3]        = 1.0f;

	ClearValue clear = { .color = { 0.0f, 0.0f, 0.0f, 0.0f } };
	std::string label = "SelectionOutline";
	commandBuffer->BeginEvent(label.c_str(), label.size() + 1);
	commandBuffer->BeginRenderTarget(outputTarget, &clear);
	commandBuffer->SetPipeline(pipeline);
	commandBuffer->SetDescriptorSet(descriptorSet);
	commandBuffer->PushConstants(ShaderStage::Vertex | ShaderStage::Pixel, &pc, sizeof(pc), 0);
	commandBuffer->DrawInstanced(3, 1, 0, 0);
	commandBuffer->EndRenderTarget();
	commandBuffer->EndEvent();
}

void SelectionOutlineTask::SetViewportSize(const Vector2 &size)
{
	if (size.x <= 0.0f || size.y <= 0.0f)
	{
		return;
	}

	if (outputTarget)
	{
		Graphics::ReleaseResource(outputTarget);
	}

	auto device = Graphics::GetDevice();
	Format colorFormat = Format::RGBA8;
	outputTarget = device->CreateRenderTarget((uint32_t)size.x, (uint32_t)size.y, &colorFormat, 1);
}

}
