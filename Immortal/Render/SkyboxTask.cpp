#include "SkyboxTask.h"
#include "FrameGraphDebugUi.h"
#include "Vision/Image.h"

#include <imgui.h>

namespace Immortal
{

SkyboxTask::SkyboxTask() :
    RenderTask{"Skybox"}
{

}

SkyboxTask::~SkyboxTask()
{
	skybox.Reset();
	Graphics::ReleaseResource(texture);
	Graphics::ReleaseResource(textureCube);
	Graphics::ReleaseResource(descriptorSet);
	Graphics::ReleaseResource((Ref<Pipeline> &)graphicsPipeline);
}

void SkyboxTask::Build(AsyncComputeThread *asyncComputeThread)
{
	if (!skybox)
	{
		skybox = Mesh::CreateCube(asyncComputeThread, nullptr, 1.0f, true);
	}

	asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
        if (filepath.empty())
        {
            return;
        }

        Picture picture = Vision::Read(filepath);
        if (!picture)
        {
            LOG::ERR("Failed to read {}", filepath.c_str());
            return;
        }

        auto device = Graphics::GetDevice();
        texture = Graphics::CreateTexture(picture, nullptr);
        texture->SetDebugName(filepath.c_str());

        auto &width = texture->GetWidth();
        auto &height = texture->GetHeight();

        textureCube = device->CreateTexture(Format::R16G16B16A16_SFLOAT, height, height, Texture::CalculateMipmapLevels(height, height), 6, TextureType::Sampled | TextureType::Storage);
		textureCube->SetName("SkyboxCube");

        pipeline = Graphics::GetPipeline("equirect2cube");
        descriptorSet = device->CreateDescriptorSet(pipeline);
		sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp, CompareOperation::Never, 0.0f, 16.0f);

        uint32_t stride = SLALIGN(picture.GetStride(0), TextureAlignment);
        size_t size = stride * picture.GetHeight();
        buffer = Graphics::GetCachedBuffer(BufferType::TransferSource, size);
        Graphics::MemoryCopyImage(buffer, stride, picture.GetData(), picture.GetStride(), picture.GetFormat(), width, height);

        commandBuffer->CopyBufferToImage(texture, 0, buffer, stride);
		commandBuffer->GenerateMipMaps(texture, Filter::Linear);
        commandBuffer->SetImageLayout(textureCube, ImageLayout::General, PipelineStage::All, PipelineStage::All);

        descriptorSet->Set(0, texture);
        descriptorSet->Set(1, textureCube);
        descriptorSet->Set(2, sampler);

        commandBuffer->SetImageLayout(textureCube, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);

        Ref<Shader> vertexShader = Graphics::GetShaderByName("skybox_VS", ShaderStage::Vertex, "VSMain");
        Ref<Shader> pixelShader  = Graphics::GetShaderByName("skybox_PS", ShaderStage::Pixel, "PSMain");
        if (!vertexShader || !pixelShader)
        {
            return;
        }
        Shader *shaders[] = {
            vertexShader,
            pixelShader
        };
        graphicsPipeline = device->CreateGraphicsPipeline();
        InputElementDescription inputElements;
        graphicsPipeline->Construct(
            shaders,
            2,
            inputElements,
            {
                Format::R16G16B16A16_SFLOAT,
                Format::R32G32_UINT,
            }
        );

        skyboxDescriptorSet = device->CreateDescriptorSet(graphicsPipeline);
		skyboxDescriptorSet->Set(0, textureCube);
        skyboxDescriptorSet->Set(1, sampler    );
    });

    asyncComputeThread->Execute<ExecutionCompletedTask>([=, this] {
        Graphics::ReleaseCachedBuffer(BufferType::TransferSource, buffer);
        buffer = {};
    });
}

void SkyboxTask::Execute(CommandBuffer *commandBuffer, const SceneParameters &params)
{
    if (!textureCube)
    {
        return;
    }

    uint32_t cubemapSize = textureCube->GetWidth();
    commandBuffer->SetImageLayout(textureCube, ImageLayout::General, PipelineStage::All, PipelineStage::All);

    commandBuffer->SetPipeline(pipeline);
    commandBuffer->SetDescriptorSet(descriptorSet);
	commandBuffer->PushConstants(ShaderStage::Compute, &cubemapSize, sizeof(cubemapSize), 0);
	commandBuffer->Dispatch(SLALIGN(cubemapSize/16, 16), SLALIGN(cubemapSize/16, 16), 6);
	commandBuffer->GenerateMipMaps(textureCube, Filter::Linear);
	commandBuffer->SetImageLayout(textureCube, ImageLayout::ShaderResource, PipelineStage::All, PipelineStage::All);
}

void SkyboxTask::Composite(CommandBuffer * commandBuffer, const SceneParameters &params)
{
    if (!textureCube)
    {
        return;
    }

    commandBuffer->SetPipeline(graphicsPipeline);
    commandBuffer->SetDescriptorSet(skyboxDescriptorSet); 

    Matrix4 invSkyboxVP = Vector::Inverse(params.skyboxProjection);
    commandBuffer->PushConstants(ShaderStage::Vertex | ShaderStage::Pixel, &invSkyboxVP, sizeof(invSkyboxVP), 0);
    commandBuffer->DrawInstanced(3, 1, 0, 0);
}

void SkyboxTask::OnFrameGraphDebugGui()
{
	ImGui::TextUnformatted("Execute: equirect→cubemap + mips. Composite: sky draw (expects Execute pass first).");
	if (textureCube)
	{
		FrameGraphDebugTextureThumbnail(textureCube.Get(), "Radiance cubemap");
	}
	if (texture)
	{
		FrameGraphDebugTextureThumbnail(texture.Get(), "Source HDR (equirect)");
	}
}

void SkyboxTask::SetFilePath(const String &value)
{
    filepath = value;
}

}
