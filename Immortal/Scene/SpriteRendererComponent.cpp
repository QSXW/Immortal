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

void SpriteRendererComponent::UpdateSprite(const Vision::Picture &picture)
{
    Format targetFormat = Format::RGBA8;
    Format lumaFormat   = Format::R8;
	Format chromaFormat = Format::RG8;
   
    if (picture.desc.format.IsType(Format::_10Bits))
	{
		lumaFormat   = Format::R16;
		chromaFormat = Format::RG16;
		targetFormat = Format::RGBA16;
	}

	uint32_t width  = picture.GetWidth();
	uint32_t height = picture.GetHeight();
	SamplingFactor samplingFactor{picture.GetFormat()};
	uint32_t chromaWidth     = width  >> samplingFactor.x;
	uint32_t chromaHeight    = height >> samplingFactor.y;
	uint32_t lumaRowPitch    = SLALIGN(picture.GetLineSize(0), TextureAlignment);
	uint32_t chromatRowPitch = SLALIGN(picture.GetLineSize(1), TextureAlignment);
	size_t totalSize = 0;
	size_t componentSize[3] = {};
	auto lumaSize = lumaRowPitch * picture.GetHeight();
	auto chromaSize = chromatRowPitch * picture.GetHeight();
	totalSize = lumaSize + chromaSize * 2;

	if (!Sprite || width != Sprite->GetWidth() || height != Sprite->GetHeight())
    {
		Graphics::DiscardTexture(Sprite);
		auto device = Graphics::GetDevice();
		Sprite   = device->CreateTexture(targetFormat, width, height, Texture::CalculateMipmapLevels(picture.GetWidth(), picture.GetHeight()), 1, TextureType::Storage);
		input[0] = device->CreateTexture(lumaFormat, width, height, 1, 1, TextureType::TransferDestination);
		if (picture.GetFormat().IsType(Format::NV))
		{
			pipeline      = Graphics::GetPipeline("color_space_nv122rgba");
			descriptorSet = device->CreateDescriptorSet(pipeline);
			input[1]      = device->CreateTexture(chromaFormat, chromaWidth, chromaHeight, 1, 1, TextureType::TransferDestination);
			descriptorSet->Set(0, input[0]);
			descriptorSet->Set(1, input[1]);
			descriptorSet->Set(2, Sprite  );
		}
		else
		{
			pipeline      = Graphics::GetPipeline("color_space_yuvp2rgba");
			descriptorSet = device->CreateDescriptorSet(pipeline);
			input[1]      = device->CreateTexture(lumaFormat, chromaWidth, chromaHeight, 1, 1, TextureType::TransferDestination);
			input[2]      = device->CreateTexture(lumaFormat, chromaWidth, chromaHeight, 1, 1, TextureType::TransferDestination);
			descriptorSet->Set(0, input[0]);
			descriptorSet->Set(1, input[1]);
			descriptorSet->Set(2, input[2]);
			descriptorSet->Set(3, Sprite  );
		}

		if (picture.shared->type == Vision::PictureType::System)
		{
			buffer = device->CreateBuffer(totalSize, BufferType::TransferSource);
		}
	}

	if (picture.shared->type == Vision::PictureType::Device)
	{
#ifdef _WIN32
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
#endif
	}
	else
	{
		uint8_t *data = nullptr;
		buffer->Map((void **) &data, totalSize, 0);
		Graphics::MemoryCopyImage(data,                         lumaRowPitch,    picture[0], picture.GetLineSize(0), lumaFormat, width, height);
		Graphics::MemoryCopyImage(data + lumaSize,              chromatRowPitch, picture[1], picture.GetLineSize(1), lumaFormat, chromaWidth, chromaHeight); 
		Graphics::MemoryCopyImage(data + lumaSize + chromaSize, chromatRowPitch, picture[2], picture.GetLineSize(2), lumaFormat, chromaWidth, chromaHeight); 
		buffer->Unmap();
	}

	Graphics::Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
		if (picture.shared->type == Vision::PictureType::Device)
		{
			commandBuffer->CopyPlatformSpecificSubresource(input[0], 0, (ID3D12Resource *) picture[0], 0);
			commandBuffer->CopyPlatformSpecificSubresource(input[1], 0, (ID3D12Resource *) picture[0], 1);
		}
		else
		{
			commandBuffer->CopyBufferToImage(input[0], 0, buffer, lumaRowPitch, 0                    );
			commandBuffer->CopyBufferToImage(input[1], 0, buffer, chromatRowPitch, lumaSize);
			commandBuffer->CopyBufferToImage(input[2], 0, buffer, chromatRowPitch, lumaSize + chromaSize);
		}
	});
	
	Graphics::Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
		commandBuffer->SetPipeline(pipeline);
		if (picture.shared->type == Vision::PictureType::System)
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
			    .samplingFactor = {sampling[samplingFactor.x], sampling[samplingFactor.y]},
			    .nomalizedFactor = picture.GetFormat().IsType(Format::_10Bits) ? 65535.0f / 1023.0f : 1.0f,
			};
			commandBuffer->PushConstants(ShaderStage::Compute, &pushConstant, sizeof(pushConstant), 0);
		}
		commandBuffer->SetDescriptorSet(descriptorSet);
		commandBuffer->Dispatch(SLALIGN(picture.GetLineSize(0) / lumaFormat.ComponentCount() / 32, 32), SLALIGN(picture.GetHeight() / 32, 32), 1);
		commandBuffer->GenerateMipMaps(Sprite, Filter::Linear);
	});

	Graphics::Execute<ExecutionCompletedTask>([picture, this]() {});
}

}
