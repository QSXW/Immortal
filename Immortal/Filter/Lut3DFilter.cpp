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

static inline const char *GetShaderName(Lut3DFilter::Type type)
{
    switch (type)
    {
        case Lut3DFilter::Type::Nearest:
            return "lut_nearest";

        case Lut3DFilter::Type::Trilinear:
            return "lut_trilinear";

        default:
            return nullptr;
    }
}

static bool LoadCube(Device *device, const String &path, Ref<Buffer> &stagingLut, int &lutSize)
{
	std::fstream file(path, std::ios::in);
	if (!file.is_open())
    {
        return false;
    }

    Vector3 min = { 0.0f, 0.0f, 0.0f };
    Vector3 max = { 1.0f, 1.0f, 1.0f};

    char line[kMaxLineSize];
	while (file.getline(line, kMaxLineSize))
    {
        if (!strncmp(line, "LUT_3D_SIZE", 11))
        {
            int size   = std::atoi(line + 12);
            if (size < 2 || size > 256)
            {
                LOG::ERR("unsupported LUT_3D_SIZE - `{}`", size);
				return false;
            }
			int size2d = size * size;

            lutSize = size;

            size_t lutWidth = size2d * size;
			stagingLut = device->CreateBuffer(BufferType::TransferSource, SLALIGN(lutWidth * sizeof(Vector3), TextureAlignment));
			if (!stagingLut)
			{
				return false;
			}

			Vector3 *data = nullptr;
            stagingLut->Map((void **) &data, stagingLut->GetSize(), 0);
			if (!data)
			{
				stagingLut = {};
				return false;
			}
            for (int k = 0; k < size; k++)
            {
                for (int j = 0; j < size; j++)
                {
                    for (int i = 0; i < size; i++)
                    {
                        ParseResult ret = ParseResult::Value;
                        do
                        {
                            ret = ParseText(file, line, min, max);
                            if (ret == ParseResult::InvalidData)
                            {
								stagingLut->Unmap();
								stagingLut = {};
                                return false;
                            }
                        } while (ret == ParseResult::Text);

                        Vector3 *rgb = &data[i * size2d + j * size + k];
                        if (sscanf(line, "%f %f %f", &rgb->r, &rgb->g, &rgb->b) != 3)
                        {
							stagingLut->Unmap();
							stagingLut = {};
                            return false;
                        }
                    }
                }
            }
            stagingLut->Unmap();
        }
    }

    return true;
}

Lut3DFilter::Lut3DFilter(Device *device, const String &filepath, Type type) :
    FilterNode{},
    device{ device },
    type{ type },
    lutSize{}
{
	auto entryPoint = GetEntryPoint(type);
	auto shaderName = GetShaderName(type);
	if (!entryPoint || !shaderName)
	{
		LOG::ERR("Incorrect lut3d type specified - `{}`", uint32_t(type));
		return;
	}

	pipeline = Graphics::GetPipeline(shaderName);
	if (!pipeline)
	{
		URef<Shader> shader = Graphics::CreateShaderByName(shaderName, entryPoint);
		if (!shader)
		{
			return;
		}
        pipeline = device->CreateComputePipeline(shader);
		if (!pipeline)
		{
			return;
		}
		Graphics::StorePipeline(shaderName, pipeline);
    }
    descriptorSet = device->CreateDescriptorSet(pipeline);
	if (!descriptorSet)
	{
		return;
	}

	if (!LoadLutFile(filepath))
	{
		return;
	}
}

Lut3DFilter::~Lut3DFilter()
{

}

bool Lut3DFilter::LoadLutFile(const String &filepath)
{
	if (!pipeline || !descriptorSet)
	{
		return false;
	}

	Ref<Buffer> nextStagingLut;
	int nextLutSize = 0;
	if (!LoadCube(device, filepath, nextStagingLut, nextLutSize) || !nextStagingLut || nextLutSize < 2)
	{
		return false;
	}

	stagingLut = nextStagingLut;
	lutSize = nextLutSize;
	return true;
}

bool Lut3DFilter::Ready() const
{
	return pipeline && descriptorSet && lutSize >= 2 && (stagingLut || lut);
}

void Lut3DFilter::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
	if (!Ready() || input.size() != 1 || !input[0])
    {
		return;
    }

    auto width  = input[0]->GetWidth();
    auto height = input[0]->GetHeight();
	if (output.empty() ||
        output[0]->GetWidth() != input[0]->GetWidth() ||
	    output[0]->GetHeight() != input[0]->GetHeight())
	{
		output.clear();
		output.emplace_back(device->CreateTexture(Format::RGBA16, width, height, Texture::CalculateMipmapLevels(width, height), 1, TextureType::Storage));
		if (!output[0])
		{
			output.clear();
			return;
		}
    }

    uint32_t slot = 0;
    for (; slot < input.size(); slot++)
    {
        descriptorSet->Set(slot, input[slot]);
    }
    descriptorSet->Set(slot++, output[0]);

    Ref<Buffer> ref;
    if (stagingLut)
    {
        ref = stagingLut;
        if (!lut || lut->GetSize() < stagingLut->GetSize())
        {
			Ref<Buffer> nextLut = device->CreateBuffer(BufferType::Storage, SLALIGN(stagingLut->GetSize(), TextureAlignment), MemoryType::Device, sizeof(Vector3));
			if (!nextLut)
			{
				output.clear();
				return;
			}
			lut = nextLut;
        }
		descriptorSet->Set(slot, lut);
		stagingLut = {};
    }

    asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
        if (ref)
        {
			commandBuffer->MemoryCopy(lut, 0, ref, 0, ref->GetSize());
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
