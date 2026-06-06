#include "SkyboxTask.h"
#include "Vision/Image.h"

namespace Immortal
{

SkyboxTask::SkyboxTask() :
    RenderTask{"Skybox"}
{

}

SkyboxTask::~SkyboxTask()
{

}

void SkyboxTask::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t value, CommandBuffer *commandBuffer) {
		skybox = Mesh::CreateCube(asyncComputeThread, commandBuffer, 1.0f, true);

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

        textureCube = device->CreateTexture(Format::R16G16B16A16_SFLOAT, height, height, Texture::CalculateMipmapLevels(height, height), 6, TextureType::Storage);
		textureCube->SetName("SkyboxCube");

        pipeline = Graphics::GetPipeline("equirect2cube");
        descriptorSet = device->CreateDescriptorSet(pipeline);
		sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);

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

        std::string shaderSource = Graphics::ReadShaderSource("Assets/Shaders/hlsl/skybox.hlsl");
        Ref<Shader> vertexShader = device->CreateShader("SkyboxVertex", ShaderStage::Vertex, shaderSource, "VSMain");
        Ref<Shader> pixelShader  = device->CreateShader("SkyboxPixel",  ShaderStage::Pixel,  shaderSource, "PSMain");
        Shader *shaders[] = {
            vertexShader,
            pixelShader
        };
        graphicsPipeline = device->CreateGraphicsPipeline();
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
    commandBuffer->PushConstants(ShaderStage::Vertex | ShaderStage::Pixel, &params.skyboxProjection, sizeof(params.skyboxProjection) + 2 * sizeof(float), 0);

    auto &nodes = skybox->NodeList();
	Buffer *vertexBuffers[] = {nodes[0].Vertex};
	commandBuffer->SetVertexBuffers(0, 1, vertexBuffers, sizeof(Mesh::SimpleVertex));
	commandBuffer->SetIndexBuffer(nodes[0].Index, Format::UINT32);
	commandBuffer->DrawIndexedInstance(nodes[0].Index->GetSize() / sizeof(uint32_t), 1, 0, 0, 0);
}

void SkyboxTask::SetFilePath(const String &value)
{
    filepath = value;
}

}
