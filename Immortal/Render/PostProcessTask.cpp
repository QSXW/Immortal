#include "PostProcessTask.h"

#include "Render/Graphics.h"

#include <cstring>

namespace Immortal
{

namespace
{

struct PostProcessConstants
{
	float bloomThreshold;
	float bloomIntensity;
	float bloomRadiusPx;
	float exposure;
	float gamma;
	float _pad[3];
};

static constexpr uint32_t kPostCbBytes = 256u;
}

PostProcessTask::PostProcessTask() :
    RenderTask{ "PostProcess" }
{
}

void PostProcessTask::SetHdrSceneColor(Texture *hdrColor)
{
	hdrColorTex = hdrColor;
}

void PostProcessTask::SetHdrPickTexture(Texture *pickTexture)
{
	hdrPickTex = pickTexture;
}

void PostProcessTask::SetViewportSize(uint32_t width, uint32_t height)
{
	viewportW = width;
	viewportH = height;
}

void PostProcessTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *) {
		auto *device = Graphics::GetDevice();
		std::string src = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / "post_hdr_present.hlsl");
		if (src.empty())
		{
			LOG::ERR("PostProcessTask: failed to read post_hdr_present.hlsl");
			return;
		}
		auto load = [&](ShaderStage st, const char *dxilName, const char *entry) -> Ref<Shader> {
			if (Shader *cached = Graphics::GetShaderByName(dxilName, st, std::string(entry)))
			{
				return Ref<Shader>{ cached };
			}
			return device->CreateShader(dxilName, st, src, std::string(entry));
		};
		Ref<Shader> vs = load(ShaderStage::Vertex, "post_hdr_present_VS", "VSMain");
		Ref<Shader> ps = load(ShaderStage::Pixel, "post_hdr_present_PS", "PSMain");
		if (!vs || !ps)
		{
			LOG::ERR("PostProcessTask: shader compile failed");
			return;
		}
		Shader *sh[] = { vs, ps };
		pipeline = device->CreateGraphicsPipeline();
		pipeline->Disable(Pipeline::State::Depth);
		pipeline->Construct(sh, 2, {}, { Format::R8G8B8A8_UNORM, Format::R32G32_UINT });

		linearSampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);
		constantBuffer = device->CreateBuffer(BufferType::ConstantBuffer, kPostCbBytes, MemoryType::Host);
		descriptorSet = device->CreateDescriptorSet(pipeline);
		descriptorSet->Set(0, constantBuffer);
		descriptorSet->Set(1, linearSampler);
	});
}

void PostProcessTask::Composite(CommandBuffer *commandBuffer, const SceneParameters &params)
{
	if (!pipeline || !descriptorSet || !constantBuffer || !hdrColorTex)
	{
		return;
	}
	descriptorSet->Set(2, hdrColorTex);
	if (hdrPickTex)
	{
		descriptorSet->Set(3, hdrPickTex);
	}

	PostProcessConstants cb{};
	cb.bloomThreshold  = 1.0f;
	cb.bloomIntensity  = 0.35f;
	cb.bloomRadiusPx   = 5.0f;
	cb.exposure        = params.exposure;
	cb.gamma           = params.gamma > 1e-4f ? params.gamma : 2.2f;

	void *mapped = nullptr;
	constantBuffer->Map(&mapped, kPostCbBytes, 0);
	std::memcpy(mapped, &cb, sizeof(cb));
	std::memset(static_cast<uint8_t *>(mapped) + sizeof(cb), 0, kPostCbBytes - sizeof(cb));
	constantBuffer->Unmap();

	commandBuffer->SetPipeline(pipeline);
	commandBuffer->SetDescriptorSet(descriptorSet);
	commandBuffer->DrawInstanced(3, 1, 0, 0);
}

}
