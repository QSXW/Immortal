/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Lut3DFilter.h"

namespace Immortal
{

constexpr size_t kMaxLineSize = 512;
enum class ParseResult
{
    InvalidData,
    Value,
    Text
};

static inline bool IsSpace(int c)
{
    return c == ' ' ||
           c == '\f' ||
           c == '\n' ||
           c == '\r' ||
           c == '\t' ||
           c == '\v';
}

static int SkipLine(const char *p)
{
    while (*p && IsSpace(*p))
    {
        p++;
    }
    return !*p || *p == '#';
}

static ParseResult ParseText(std::fstream &stream, char *line, Vector3 &minValues, Vector3 &maxValues)
{
    do
    {
        stream.getline(line, kMaxLineSize);
        if (!strncmp(line, "DOMAIN_", 7))
        {
            Vector3 *v = NULL;
            if (!strncmp(line + 7, "MIN ", 4))
            {
                v = &minValues;
            }
            else if (!strncmp(line + 7, "MAX ", 4))
            {
                v = &maxValues;
            }
            if (!v)
            {
                return ParseResult::InvalidData;
            }
            if (sscanf(line + 11, "%f %f %f", &v->r, &v->g, &v->b) != 3)
            {
                return ParseResult::InvalidData;
            }

            return ParseResult::Text;
        }
        else if (!strncmp(line, "TITLE", 5))
        {
            return ParseResult::Text;
        }
    } while (SkipLine(line));

    return ParseResult::Value;
}

static inline const char *GetEntryPoint(Lut3DFilter::Type type)
{
    switch (type)
    {
        case Lut3DFilter::Type::Nearest:
            return "InterpolateNearest";

        case Lut3DFilter::Type::Trilinear:
            return "InterpolateTrilinear";

        default:
            return nullptr;
    }
}

Lut3DFilter::Lut3DFilter(Device *device, const String &filepath, Type type, uint32_t width, uint32_t height) :
    FilterNode{},
    type{ type },
    lutSize{}
{	
    std::fstream lutFileStream(filepath.GetWString());
    if (lutFileStream.bad())
    {
        return;
    }

    Vector3 min = { 0.0f, 0.0f, 0.0f };
    Vector3 max = { 1.0f, 1.0f, 1.0f};

    char line[kMaxLineSize];
    while (lutFileStream.getline(line, kMaxLineSize))
    {
        if (!strncmp(line, "LUT_3D_SIZE", 11))
        {
            int size   = std::atoi(line + 12);
            int size2d = size * size;
            if (size < 2 || size > 256)
            {
                LOG::ERR("unsupported LUT_3D_SIZE - `{}`", size);
            }

            lutSize = size;

            size_t lutWidth = size2d * size;
            stagingLut = device->CreateBuffer(SLALIGN(lutWidth * sizeof(Vector3), TextureAlignment), BufferType::TransferSource);
            lut        = device->CreateBuffer(SLALIGN(lutWidth * sizeof(Vector3), TextureAlignment), BufferType::ConstantBuffer, Format::R32G32B32_SFLOAT);

            Vector3 *data;
            stagingLut->Map((void **) &data, stagingLut->GetSize(), 0);
            for (int k = 0; k < size; k++)
            {
                for (int j = 0; j < size; j++)
                {
                    for (int i = 0; i < size; i++)
                    {
                        ParseResult ret = ParseResult::Value;
                        do
                        {
                            ret = ParseText(lutFileStream, line, min, max);
                            if (ret == ParseResult::InvalidData)
                            {
                                return;
                            }
                        } while (ret == ParseResult::Text);

                        Vector3 *rgb = &data[i * size2d + j * size + k];				
                        if (sscanf(line, "%f %f %f", &rgb->r, &rgb->g, &rgb->b) != 3)
                        {
                            return;
                        }
                    }
                }
            }
            stagingLut->Unmap();
        }
    }

    Stream stream{ "Assets/Shaders/hlsl/lut.hlsl", StreamMode::Read };
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
        LOG::ERR("Incorrect lut3d type specificed - `{}`", uint32_t(type));
        return;
    }

    URef<Shader> shader = device->CreateShader("Lut3D", ShaderStage::Compute, source, entryPoint);

    pipeline = device->CreateComputePipeline(shader);
    descriptorSet = device->CreateDescriptorSet(pipeline);

    output.emplace_back(device->CreateTexture(Format::RGBA16, width, height, Texture::CalculateMipmapLevels(width, height), 1, TextureType::Storage));
}

Lut3DFilter::~Lut3DFilter()
{

}

void Lut3DFilter::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
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
    descriptorSet->Set(slot++, output[0]);
    descriptorSet->Set(slot,   lut      );

    Ref<Buffer> ref;
    if (stagingLut)
    {
        ref = stagingLut;
    }

    asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
        if (stagingLut)
        {
            commandBuffer->MemoryCopy(lut, 0, stagingLut, 0, lut->GetSize());
            stagingLut.Reset();
        }

        uint32_t nThreadX = SLALIGN(output[0]->GetWidth() / 32, 32);
        uint32_t nThreadY = SLALIGN(output[0]->GetHeight() / 32, 32);

        struct
        {
            int lutSize;
            int lutSize2;
            int lutSize3;
        } pushConstant = {
            .lutSize  = lutSize,
            .lutSize2 = lutSize * lutSize,
            .lutSize3 = pushConstant.lutSize2 * lutSize
        };

        commandBuffer->SetPipeline(pipeline);
        commandBuffer->SetDescriptorSet(descriptorSet);
        commandBuffer->PushConstants(ShaderStage::Compute, &pushConstant, sizeof(pushConstant), 0);
        commandBuffer->Dispatch(nThreadX, nThreadY, 1);
    });

    if (ref)
    {
        asyncComputeThread->Execute<ExecutionCompletedTask>([ref] {});
    }
}

}
