#include "EditorGridTask.h"
#include "Graphics/AsyncCompute.h"

namespace Immortal
{

namespace
{

struct GridPushConstants
{
	Matrix4 invViewProjection;
	Matrix4 viewProjection;
	Vector4 cameraWorld;
	Vector4 config;
};

}

EditorGridTask::EditorGridTask() = default;

EditorGridTask::~EditorGridTask() = default;

void EditorGridTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
		auto device = Graphics::GetDevice();

		std::string srcUnder = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "editor_grid_underlay.hlsl");
		std::string srcPost = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "editor_grid_post.hlsl");
		if (srcUnder.empty() || srcPost.empty())
		{
			LOG::ERR("EditorGridTask: failed to read editor_grid_underlay.hlsl or editor_grid_post.hlsl");
			return;
		}

		bindGuardTexture = Graphics::Preset()->Textures.Black;

		Ref<Shader> vsUnder = device->CreateShader("EditorGridVSUnder", ShaderStage::Vertex, srcUnder, "VSMain");
		Ref<Shader> psUnder = device->CreateShader("EditorGridPSUnder", ShaderStage::Pixel, srcUnder, "PSMain");
		Ref<Shader> vsPost = device->CreateShader("EditorGridVSPost", ShaderStage::Vertex, srcPost, "VSMain");
		Ref<Shader> psPost = device->CreateShader("EditorGridPSPost", ShaderStage::Pixel, srcPost, "PSMain");
		if (!vsUnder || !psUnder || !vsPost || !psPost)
		{
			LOG::ERR("EditorGridTask: shader compile failed");
			return;
		}

		Shader *shadersUnder[] = { vsUnder, psUnder };
		underlayPipeline = device->CreateGraphicsPipeline();
		underlayPipeline->Enable(Pipeline::State::Blend);
		underlayPipeline->Disable(Pipeline::State::Depth);
		underlayPipeline->Construct(
		    shadersUnder,
		    SL_ARRAY_LENGTH(shadersUnder),
		    {},
		    {
		        Format::R8G8B8A8_UNORM,
		        Format::R32G32_UINT,
		        Format::Depth24Stencil8,
		    });

		Shader *shadersPost[] = { vsPost, psPost };
		postPipeline = device->CreateGraphicsPipeline();
		postPipeline->Disable(Pipeline::State::Depth);
		postPipeline->Construct(shadersPost, SL_ARRAY_LENGTH(shadersPost), {}, { Format::R8G8B8A8_UNORM });

		linearSampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);

		underlayDescriptorSet = device->CreateDescriptorSet(underlayPipeline);
		underlayDescriptorSet->Set(0, bindGuardTexture);
		underlayDescriptorSet->Set(1, linearSampler);

		postDescriptorSet = device->CreateDescriptorSet(postPipeline);
		postDescriptorSet->Set(2, linearSampler);

		built = true;
	});
}

static void FillGridPush(GridPushConstants &pc, const Matrix4 &invViewProjection, const Matrix4 &viewProjection, const Vector4 &cameraWorld)
{
	pc.invViewProjection = invViewProjection;
	pc.viewProjection    = viewProjection;
	pc.cameraWorld       = cameraWorld;
	pc.config            = Vector4{ 1.0f, 10.0f, 30.0f, 4.0f };
}

void EditorGridTask::ExecuteUnderlay(CommandBuffer *commandBuffer, const Matrix4 &invViewProjection, const Matrix4 &viewProjection, const Vector4 &cameraWorld)
{
	if (!built || !underlayPipeline || !underlayDescriptorSet)
	{
		return;
	}

	GridPushConstants pc{};
	FillGridPush(pc, invViewProjection, viewProjection, cameraWorld);

	std::string label = "EditorGridUnderlay";
	commandBuffer->BeginEvent(label.c_str(), label.size() + 1);
	commandBuffer->SetPipeline(underlayPipeline);
	commandBuffer->SetDescriptorSet(underlayDescriptorSet);
	commandBuffer->PushConstants(ShaderStage::Vertex | ShaderStage::Pixel, &pc, sizeof(pc), 0);
	commandBuffer->DrawInstanced(3, 1, 0, 0);
	commandBuffer->EndEvent();
}

void EditorGridTask::ExecutePost(CommandBuffer *commandBuffer, Texture *sceneColor, Texture *depthTexture, const Matrix4 &invViewProjection, const Matrix4 &viewProjection, const Vector4 &cameraWorld)
{
	if (!built || !postPipeline || !postDescriptorSet || !postOutputTarget || !sceneColor || !depthTexture)
	{
		return;
	}

	postDescriptorSet->Set(0, sceneColor);
	postDescriptorSet->Set(1, depthTexture);

	GridPushConstants pc{};
	FillGridPush(pc, invViewProjection, viewProjection, cameraWorld);

	ClearValue clear = { .color = { 0.0f, 0.0f, 0.0f, 0.0f } };
	std::string label = "EditorGridPost";
	commandBuffer->BeginEvent(label.c_str(), label.size() + 1);
	commandBuffer->BeginRenderTarget(postOutputTarget, &clear);
	commandBuffer->SetPipeline(postPipeline);
	commandBuffer->SetDescriptorSet(postDescriptorSet);
	commandBuffer->PushConstants(ShaderStage::Vertex | ShaderStage::Pixel, &pc, sizeof(pc), 0);
	commandBuffer->DrawInstanced(3, 1, 0, 0);
	commandBuffer->EndRenderTarget();
	commandBuffer->EndEvent();
}

void EditorGridTask::SetViewportSize(const Vector2 &size)
{
	if (size.x <= 0.0f || size.y <= 0.0f)
	{
		return;
	}

	if (postOutputTarget)
	{
		Graphics::ReleaseResource(postOutputTarget);
	}

	auto device = Graphics::GetDevice();
	Format colorFormat = Format::RGBA8;
	postOutputTarget = device->CreateRenderTarget((uint32_t)size.x, (uint32_t)size.y, &colorFormat, 1);
}

}
