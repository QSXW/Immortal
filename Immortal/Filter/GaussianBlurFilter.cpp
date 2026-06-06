/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "GaussianBlurFilter.h"

namespace Immortal
{

constexpr size_t kHorizontal = 0;
constexpr size_t kVertical   = 1;
GaussianBlurFilter::GaussianBlurFilter(Device *device, float sigma, int kernalSize, float verticalSigma, int verticalKernalSize) :
    FilterNode{},
    device{device}
{
    std::string source = Graphics::ReadShaderSource("Assets/Shaders/hlsl/GaussianBlur.hlsl");
    if (source.empty())
    {
        return;
    }

    CalculateGaussianKernal(device, stagingKernalBuffer[kHorizontal], sigma, kernalSize);
    kernal[kHorizontal] = device->CreateBuffer(stagingKernalBuffer[kHorizontal]->GetSize(), BufferType::ConstantBuffer, Format::R32_SFLOAT);

    if (!verticalSigma)
    {
        verticalSigma = sigma;
    }
    if (!verticalKernalSize)
    {
        verticalKernalSize = kernalSize;
    }

    if (verticalSigma != sigma || verticalKernalSize != kernalSize)
    {
        kernal[kVertical] = device->CreateBuffer(stagingKernalBuffer[kVertical]->GetSize(), BufferType::ConstantBuffer, Format::R32_SFLOAT);
        CalculateGaussianKernal(device, stagingKernalBuffer[kVertical], verticalKernalSize, verticalKernalSize);
    }
    else
    {
        stagingKernalBuffer[kVertical] = stagingKernalBuffer[kHorizontal];
        kernal[kVertical] = kernal[kHorizontal];
    }

    kernalSizes[kHorizontal] = kernalSize;
    kernalSizes[kVertical]   = verticalKernalSize;

    URef<Shader> shader = device->CreateShader("GaussiaBlur", ShaderStage::Compute, source, "Blur");
    if (!shader)
    {
        return;
    }

    pipelines[kHorizontal] = device->CreateComputePipeline(shader);
    pipelines[kVertical]   = device->CreateComputePipeline(shader);

    sampler = device->CreateSampler(Filter::Linear, AddressMode::Mirror);
}

void GaussianBlurFilter::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
    if (output.size() != input.size())
    {
        descriptorSets.clear();
        for (auto &texture : input)
        {
            auto &width  = texture->GetWidth();
            auto &height = texture->GetHeight();
            output.emplace_back(device->CreateTexture(texture->GetFormat(), width, height, Texture::CalculateMipmapLevels(width, height), 1, TextureType::Storage));
            temp.emplace_back(device->CreateTexture(texture->GetFormat(), width, height, Texture::CalculateMipmapLevels(width, height), 1, TextureType::Storage));
            descriptorSets.emplace_back(device->CreateDescriptorSet(pipelines[kHorizontal]));
            descriptorSets.emplace_back(device->CreateDescriptorSet(pipelines[kHorizontal]));
        }
    }

    for (size_t i = 0; i < input.size(); i++)
    {
        descriptorSets[i]->Set(0, input[i]);
        descriptorSets[i]->Set(1, temp[i]);
        descriptorSets[i]->Set(2, kernal[kHorizontal]);
        descriptorSets[i]->Set(3, sampler);

        descriptorSets[input.size() + i]->Set(0, temp[i]);
        descriptorSets[input.size() + i]->Set(1, output[i]);
        descriptorSets[input.size() + i]->Set(2, kernal[kVertical]);
        descriptorSets[input.size() + i]->Set(3, sampler);
    }

    asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
        commandBuffer->MemoryCopy(kernal[kHorizontal], 0, stagingKernalBuffer[kHorizontal], 0, stagingKernalBuffer[kHorizontal]->GetSize());
        if (kernal[kVertical] != kernal[kHorizontal])
        {
            commandBuffer->MemoryCopy(kernal[kVertical], 0, stagingKernalBuffer[kVertical], 0, stagingKernalBuffer[kVertical]->GetSize());
        }

        for (size_t i = 0; i < input.size(); i++)
        {
            uint32_t nThreadX = SLALIGN(output[i]->GetWidth() / 16, 16);
            uint32_t nThreadY = SLALIGN(output[i]->GetHeight() / 16, 16);

            struct
            {
                int blurDirection;
                int kernalSize;
            } pushConstant;

            pushConstant.blurDirection = 0;
            pushConstant.kernalSize    = kernalSizes[kHorizontal];
            commandBuffer->SetPipeline(pipelines[kHorizontal]);
            commandBuffer->SetDescriptorSet(descriptorSets[i]);
            commandBuffer->PushConstants(ShaderStage::Compute, &pushConstant, sizeof(pushConstant), 0);
            commandBuffer->Dispatch(nThreadX, nThreadY, 1);
            commandBuffer->GenerateMipMaps(temp[i], Filter::Linear);

            pushConstant.blurDirection = 1;
            pushConstant.kernalSize = kernalSizes[kVertical];
            commandBuffer->SetPipeline(pipelines[kVertical]);
            commandBuffer->SetDescriptorSet(descriptorSets[input.size() + i]);
            commandBuffer->PushConstants(ShaderStage::Compute, &pushConstant, sizeof(pushConstant), 0);
            commandBuffer->Dispatch(nThreadX, nThreadY, 1);
        }
    });
}

void GaussianBlurFilter::CalculateGaussianKernal(Device *device, Ref<Buffer> &buffer, float sigma, int size)
{
    buffer = device->CreateBuffer(SLALIGN(size * sizeof(float), TextureAlignment), BufferType::TransferSource);

    float *kernal = nullptr;
    buffer->Map((void **)&kernal, buffer->GetSize(), 0);
    Math::GenerateGaussianKernal(kernal, size, sigma);
    buffer->Unmap();
}

}
