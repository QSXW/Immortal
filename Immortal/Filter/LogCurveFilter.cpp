/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "LogCurveFilter.h"

namespace Immortal
{

static inline const char *GetEntryPoint(LogCurveFilterNode::LogCurveType type)
{
    switch (type)
    {
        case LogCurveFilterNode::LogCurveType::Slog2:
            return "DecodeSlog2";

        case LogCurveFilterNode::LogCurveType::Slog3:
            return "DecodeSlog3";

        default:
            return nullptr;
    }
}

LogCurveFilterNode::LogCurveFilterNode(Device *device, LogCurveType type, uint32_t width, uint32_t height) :
    FilterNode{}
{
    Stream stream{ "Assets/Shaders/hlsl/log_curve.hlsl", StreamMode::Read };
    if (!stream.Readable())
    {
        LOG::ERR("Failed to open `{}`", stream.GetFilePath());
        return;
    }

    std::string source;
    stream.Read(source);

    auto entryPoint = GetEntryPoint(type);
    if (!entryPoint)
    {
        LOG::ERR("Incorrect log curve type specificed - `{}`", uint32_t(type));
        return;
    }

    URef<Shader> shader = device->CreateShader("LogCurve", ShaderStage::Compute, source, entryPoint);

    pipeline = device->CreateComputePipeline(shader);
    descriptorSet = device->CreateDescriptorSet(pipeline);

    output.emplace_back(device->CreateTexture(Format::RGBA16, width, height, Texture::CalculateMipmapLevels(width, height), 1, TextureType::Storage));
}

LogCurveFilterNode::~LogCurveFilterNode()
{

}

void LogCurveFilterNode::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
    if (output.empty())
    {
        return;
    }

    uint32_t slot = 0;
    for (; slot < input.size(); slot++)
    {
        descriptorSet->Set(slot, input[slot]);
    }
    descriptorSet->Set(slot, output[0]);

    asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
        uint32_t nThreadX = SLALIGN(output[0]->GetWidth()  / 32, 32);
        uint32_t nThreadY = SLALIGN(output[0]->GetHeight() / 32, 32);

        commandBuffer->SetPipeline(pipeline);
        commandBuffer->SetDescriptorSet(descriptorSet);
        commandBuffer->Dispatch(nThreadX, nThreadY, 1);
    });
}

}
