/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "DisplayOrientationFilter.h"

namespace Immortal
{

DisplayOrientationFilter::DisplayOrientationFilter(Device *device, bool hflip, bool vflip, int anticlockwiseRotation) :
    FilterNode{},
    device{ device },
    hflip{ hflip },
    vflip{ vflip },
    anticlockwiseRotation{ anticlockwiseRotation }
{
	if (anticlockwiseRotation == 180)
	{
		this->hflip = hflip ^ true;
		this->vflip = vflip ^ true;
	}
}

DisplayOrientationFilter::DisplayOrientationFilter(Device *device, int exifOrientaiton) :
    FilterNode{},
    device{},
    hflip{},
    vflip{},
    anticlockwiseRotation{}
{

}

void DisplayOrientationFilter::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
	if (!hflip && !vflip && anticlockwiseRotation == 0)
	{
		output = input;
		return;
	}

	if (output.empty())
	{
		Stream stream{"Assets/Shaders/hlsl/flip.hlsl", StreamMode::Read};
		if (!stream.Readable())
		{
			LOG::ERR("Failed to open `{}`", stream.GetFilePath());
			return;
		}

		std::string source;
		stream.Read(source);

		uint32_t numMacro = 0;
		ShaderMacro macros[4] = {};
		if (hflip)
		{
			macros[numMacro++] = { "HFLIP", nullptr };
		}
		if (vflip)
		{
			macros[numMacro++] = { "VFLIP", nullptr };
		}
		if (anticlockwiseRotation == 90)
		{
			macros[numMacro++] = { "TRANSPOSE", nullptr };
		}

		URef<Shader> shader = device->CreateShader("Flip", ShaderStage::Compute, source, "Flip", macros, numMacro);
		pipeline = device->CreateComputePipeline(shader);

		for (size_t i = 0; i < input.size(); i++)
		{
			uint32_t width  = input[i]->GetWidth();
			uint32_t height = input[i]->GetHeight();

			if (anticlockwiseRotation == 90 || anticlockwiseRotation == 270)
			{
				std::swap(width, height);
			}

			output.emplace_back(device->CreateTexture(input[i]->GetFormat(), width, height, Texture::CalculateMipmapLevels(width, height), 1, TextureType::Storage));
			descriptorSets.emplace_back(device->CreateDescriptorSet(pipeline));
			descriptorSets[i]->Set(1, output[i]);
		}
	}

	for (size_t i = 0; i < input.size(); i++)
	{
		descriptorSets[i]->Set(0, input[i]);
	}

	asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
		for (size_t i = 0; i < input.size(); i++)
		{
			uint32_t nThreadX = SLALIGN(output[i]->GetWidth()  / 32, 32);
			uint32_t nThreadY = SLALIGN(output[i]->GetHeight() / 32, 32);
			commandBuffer->SetPipeline(pipeline);
			commandBuffer->SetDescriptorSet(descriptorSets[i]);
			commandBuffer->Dispatch(nThreadX, nThreadY, 1);
		}
	});
}

}
