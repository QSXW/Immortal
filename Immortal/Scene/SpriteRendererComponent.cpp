/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Component.h"
#include "Vision/Common/SamplingFactor.h"

namespace Immortal
{

SpriteRendererComponent::SpriteRendererComponent()
{

}

SpriteRendererComponent::~SpriteRendererComponent()
{

}

void GetSamplingFactor(Format::ValueType format, SamplingFactor *factors)
{
	switch (format)
	{
		case Format::YUV444P:
		case Format::YUV444P10:
		case Format::YUV444P12:
		case Format::YUV444P16:
			factors[0] = {};
			factors[1] = {};
			factors[2] = {};
			break;

		case Format::YUV422P:
		case Format::YUV422P10:
		case Format::YUV422P12:
		case Format::YUV422P16:
			factors[0] = {};
			factors[1] = { 1, 0 };
			factors[2] = { 1, 0 };
			break;

		case Format::YUV420P:
		case Format::YUV420P10:
		case Format::YUV420P12:
		case Format::YUV420P16:
		case Format::NV12:
		case Format::P010LE:
			factors[0] = {};
			factors[1] = { 1, 1 };
			factors[2] = { 1, 1 };
			break;
		case Format::Y210:
		case Format::Y216:
			factors[0] = { 1, 0 };
			break;

		default:
			break;
	}
}

void SpriteRendererComponent::UpdateSprite(const Vision::Picture &picture)
{
    Format targetFormat = Format::RGBA8;
    Format lumaFormat   = Format::R8;
	Format chromaFormat = Format::RG8;
   
	auto &format = picture.GetFormat();
	if (format.IsType(Format::_10Bits))
	{
		lumaFormat   = Format::R16;
		chromaFormat = Format::RG16;
		targetFormat = Format::RGBA16;
	}
	if (format == Format::Y210)
	{
		lumaFormat = Format::RGBA16;
	}

	struct
	{
		uint32_t width;
		uint32_t height;
		uint32_t rowPitch;
		size_t   size;
		Format   format;
	} data[3];

	SamplingFactor factors[3] = {};
	GetSamplingFactor(format, factors);

	size_t totalSize = 0;

	for (size_t i = 0; picture[i]; i++)
	{
		data[i].format   = lumaFormat;
		if (i > 0 && format.IsType(Format::NV))
		{
			data[i].format = chromaFormat;
		}
		data[i].width    = picture.GetWidth()  >> factors[i].x;
		data[i].height   = picture.GetHeight() >> factors[i].y;
		data[i].rowPitch = SLALIGN(picture.GetStride(i), TextureAlignment);
		data[i].size     = data[i].rowPitch * data[i].height;
		totalSize += data[i].size;
	}

	uint32_t width  = picture.GetWidth();
	uint32_t height = picture.GetHeight();
	if (!Sprite || width != Sprite->GetWidth() || height != Sprite->GetHeight())
    {
		Graphics::DiscardTexture(Sprite);
		auto device = Graphics::GetDevice();
		Sprite   = device->CreateTexture(targetFormat, width, height, Texture::CalculateMipmapLevels(picture.GetWidth(), picture.GetHeight()), 1, TextureType::Storage);
		if (format.IsType(Format::NV))
		{
			pipeline      = Graphics::GetPipeline("color_space_nv122rgba");
			descriptorSet = device->CreateDescriptorSet(pipeline);
		}
		else if (picture.GetFormat() == Format::Y210)
		{
			pipeline = Graphics::GetPipeline("color_space_y2102rgba");
			descriptorSet = device->CreateDescriptorSet(pipeline);
		}
		else
		{
			pipeline      = Graphics::GetPipeline("color_space_yuvp2rgba");
			descriptorSet = device->CreateDescriptorSet(pipeline);
		}

		for (size_t i = 0; picture[i]; i++)
		{
			input[i] = device->CreateTexture(data[i].format, data[i].width, data[i].height, 1, 1, TextureType::TransferDestination);
		}

		uint32_t slot = 0;
		for (; picture[slot]; slot++)
		{
			descriptorSet->Set(slot, input[slot]);
		}
		descriptorSet->Set(slot, Sprite);

		if (picture.GetMemoryType() == Vision::PictureMemoryType::System)
		{
			buffer = device->CreateBuffer(totalSize, BufferType::TransferSource);
		}
	}

#ifdef _WIN32
	if (picture.GetMemoryType() == Vision::PictureMemoryType::Device)
	{
		ID3D12Fence *fence  = (ID3D12Fence *)picture[1];
		uint64_t fenceValue = (uint64_t)picture[2];
		Graphics::Execute<QueueTask>([=, this](Queue *_queue)
		{
			auto queue = (ID3D12CommandQueue *)_queue->GetBackendHandle();
			if (FAILED(queue->Wait(fence, fenceValue)))
			{
				LOG::ERR("Failed to wait fence `{}` and value `{}`", (void *) fence, fenceValue);
			}
		});
	}
#endif

	Graphics::Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
		if (picture.GetMemoryType() == Vision::PictureMemoryType::Device)
		{
			commandBuffer->CopyPlatformSpecificSubresource(input[0], 0, (ID3D12Resource *) picture[0], 0);
			commandBuffer->CopyPlatformSpecificSubresource(input[1], 0, (ID3D12Resource *) picture[0], 1);
		}
		else
		{
			uint8_t *mapped = nullptr;
			buffer->Map((void **) &mapped, totalSize, 0);
			size_t offset = 0;
			for (size_t i = 0; picture[i]; i++)
			{
				Graphics::MemoryCopyImage(mapped + offset, data[i].rowPitch, picture[i], picture.GetStride(i), data[i].format, data[i].width, data[i].height);
				commandBuffer->CopyBufferToImage(input[i], 0, buffer, data[i].rowPitch, offset);
				offset += data[i].size;
			}
			buffer->Unmap();
		}

		commandBuffer->SetPipeline(pipeline);

		uint32_t nThreadX = SLALIGN(data[0].width  / 32, 32);
		uint32_t nThreadY = SLALIGN(data[0].height / 32, 32);

		if (picture.GetMemoryType() == Vision::PictureMemoryType::System && !format.IsType(Format::NV) && format != Format::Y210 && format != Format::Y216)
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
			PushConstant pushConstant = {
			    .samplingFactor = {sampling[factors[1].x], sampling[factors[1].y]},
			    .nomalizedFactor = picture.GetFormat().IsType(Format::_10Bits) ? 65535.0f / 1023.0f : 1.0f,
			};
			commandBuffer->PushConstants(ShaderStage::Compute, &pushConstant, sizeof(pushConstant), 0);
		}
		commandBuffer->SetDescriptorSet(descriptorSet);
		commandBuffer->Dispatch(nThreadX, nThreadY, 1);
		commandBuffer->GenerateMipMaps(Sprite, Filter::Linear);
	});

	Graphics::Execute<ExecutionCompletedTask>([picture, this]() {});
}

}
