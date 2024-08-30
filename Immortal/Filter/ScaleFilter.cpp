/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "ScaleFilter.h"

namespace Immortal
{

ScaleFilter::ScaleFilter(Device *device, Format srcFormat, Format dstFormat, uint32_t width, uint32_t height) :
    FilterNode{},
    srcFormat{ srcFormat }
{
    output.emplace_back(device->CreateTexture(dstFormat, width, height, Texture::CalculateMipmapLevels(width, height), 1, TextureType::Storage));
    if (srcFormat.IsType(Format::NV))
    {
        pipeline = Graphics::GetPipeline("color_space_nv122rgba");
    }
    else if (srcFormat == Format::Y210)
    {
        pipeline = Graphics::GetPipeline("color_space_y2102rgba");
    }
    else
    {
        pipeline = Graphics::GetPipeline("color_space_yuvp2rgba");
    }
    descriptorSet = device->CreateDescriptorSet(pipeline);
}

ScaleFilter::~ScaleFilter()
{

}

void ScaleFilter::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
    uint32_t slot = 0;
    for (; slot < input.size(); slot++)
    {
        descriptorSet->Set(slot, input[slot]);
    }
    descriptorSet->Set(slot, output[0]);

    asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
        commandBuffer->SetPipeline(pipeline);
        uint32_t nThreadX = SLALIGN(output[0]->GetWidth()  / 32, 32);
        uint32_t nThreadY = SLALIGN(output[0]->GetHeight() / 32, 32);

        if (!srcFormat.IsType(Format::NV) && srcFormat != Format::Y210 && srcFormat != Format::Y216)
        {
            struct PushConstant
            {
                float samplingFactor[2];
                float nomalizedFactor;
            };

            static const float sampling[] = {
                1.0f,
                0.5f,
            };
            
            SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
            GetSamplingFactor(srcFormat, factors);

            PushConstant pushConstant = {
                .samplingFactor = { sampling[factors[1].x], sampling[factors[1].y ]},
                .nomalizedFactor = srcFormat.IsType(Format::_10Bits) ? 65535.0f / 1023.0f : 1.0f,
            };
            commandBuffer->PushConstants(ShaderStage::Compute, &pushConstant, sizeof(pushConstant), 0);
        }
        commandBuffer->SetDescriptorSet(descriptorSet);
        commandBuffer->Dispatch(nThreadX, nThreadY, 1);
    });
}

}
