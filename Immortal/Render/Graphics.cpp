#include "Graphics.h"
#include "Render2D.h"
#include "Framework/Timer.h"

namespace Immortal
{

URef<Graphics> Graphics::This;

Graphics::Graphics(Device *device) :
    device{ device },
    thread{ device }
{
	This = this;
	commandBuffer = device->CreateCommandBuffer(QueueType::Compute);

    {
        constexpr uint32_t white        = 0xffffffff;
        constexpr uint32_t black        = 0x000000ff;
        constexpr uint32_t transparency = 0x00000000;
        constexpr uint32_t normal       = 0xffff7f7f;

        data.Textures.White       = Graphics::CreateTexture(Format::RGBA8, 1, 1, &white       );
        data.Textures.Black       = Graphics::CreateTexture(Format::RGBA8, 1, 1, &black       );
        data.Textures.Transparent = Graphics::CreateTexture(Format::RGBA8, 1, 1, &transparency);
        data.Textures.Normal      = Graphics::CreateTexture(Format::RGBA8, 1, 1, &normal      );
    }
}

void Graphics::SetDevice(Device *device)
{
    if (!This)
    {
        new Graphics{ device };
    }
}

Device *Graphics::GetDevice()
{
	return This->device;
}

void Graphics::Release()
{
	This.Reset();
}

Graphics::~Graphics()
{
    data.Textures.White.Reset();
    data.Textures.Black.Reset();
    data.Textures.Transparent.Reset();
    data.Textures.Normal.Reset();
    
    thread.Execute<AsyncTask>(AsyncTaskType::Terminate);

    discardedTextures.clear();
    stagingBuffer.Reset();
    device = nullptr;
}

Buffer *Graphics::CreateBuffer(size_t size, BufferType type, const void *data)
{
	auto device = This->device;
	Buffer *buffer = device->CreateBuffer(size, type);

    if (data)
    {
		void *mapped = nullptr;
		buffer->Map(&mapped, size, 0);
		memcpy(mapped, data, size);
		buffer->Unmap();
    }

    return buffer;
}

Texture *Graphics::CreateTexture(const std::string &filepath)
{
	Frame frame = { filepath };
    if (!frame.Available())
    {
		return nullptr;
    }

    Picture picture = frame.GetPicture();
	uint32_t width  = picture.GetWidth();
	uint32_t height = picture.GetHeight();

    return CreateTexture(picture.GetFormat(), width, height, picture.GetData());
}

Texture *Graphics::CreateTexture(Format format, uint32_t width, uint32_t height, const void *data)
{
	uint32_t mipLevels = Texture::CalculateMipmapLevels(width, height);
	Texture *texture = This->device->CreateTexture(format, width, height, mipLevels, 1, TextureType::TransferDestination);

    if (!data)
    {
		return texture;
    }

    uint32_t uploadPitch = SLALIGN(width * format.GetComponent(), TextureAlignment);
	uint32_t uploadSize = height * uploadPitch;

    auto &buffer = This->stagingBuffer;
	if (!buffer || buffer->GetSize() < uploadSize)
    {
		buffer = This->device->CreateBuffer(uploadSize, BufferType::TransferSource);
    }

	void *mapped = nullptr;
	buffer->Map(&mapped, uploadSize, 0);
	for (int y = 0; y < height; y++)
	{
		memcpy((void *)((uintptr_t) mapped + y * uploadPitch), ((uint8_t *)data) + y * width * 4, width * 4);
	}
	buffer->Unmap();

    This->commandBuffer->Begin();
	This->commandBuffer->CopyBufferToImage(texture, 0, buffer, uploadPitch);
	This->commandBuffer->GenerateMipMaps(texture, Filter::Linear);
	This->commandBuffer->End();

    URef<Queue> queue = This->device->CreateQueue(QueueType::Compute);
	queue->Submit(This->commandBuffer);
	queue->WaitIdle(std::numeric_limits<uint32_t>::max());

    return texture;
}

void Graphics::DiscardTexture(const Ref<Texture> &texture)
{
    if (texture)
    {
		This->discardedTextures[This->index].emplace_back(texture);
    }
}

void Graphics::SetRenderIndex(uint64_t index)
{
    if (!This->discardedTextures.empty())
    {
        for (auto it = This->discardedTextures.begin(); it != This->discardedTextures.end();)
        {
			auto &[renderIndex, textures] = *it;

			if (index - renderIndex >= 3)
			{
				it = This->discardedTextures.erase(it);
				break;
			}
            else
            {
				it++;
            }
        }
    }

	This->index = index;
}

Ref<Pipeline> Graphics::GetPipeline(const std::string &name)
{
    struct ShaderCreateInfo
    {
		std::string path;
		ShaderStage stage;
		std::string entryPoint;
    };

    static const std::unordered_map<std::string, ShaderCreateInfo> pipelineShaders = {
        { "color_space_nv122rgba", { "Assets/Shaders/hlsl/color_space_nv122rgba.hlsl", ShaderStage::Compute, "main" } },
		{ "color_space_yuvp2rgba", { "Assets/Shaders/hlsl/color_space_yuvp2rgba.hlsl", ShaderStage::Compute, "main" } }
    };

	auto it = This->pipelines.find(name);
    if (it != This->pipelines.end())
    {
		return it->second;
    }

    auto shaderIt = pipelineShaders.find(name);
	if (shaderIt != pipelineShaders.end())
    {
		auto &[first, createInfo] = *shaderIt;
		Stream stream = { createInfo.path, Stream::Mode::Read };
        if (stream.Readable())
        {
			std::string source;
			stream.Read(source);
			URef<Shader> shader    = This->device->CreateShader(name, createInfo.stage, source, createInfo.entryPoint);
			Ref<Pipeline> pipeline = This->device->CreateComputePipeline(shader);
			This->pipelines[name]  = pipeline;
			return pipeline;
        }
    }

    return nullptr;
}

void Graphics::MemoryCopyImage(uint8_t *dst, uint32_t dstStride, const uint8_t *src, uint32_t srcStride, Format format, uint32_t width, uint32_t height)
{
	auto texelSize = format.GetTexelSize();
    if (srcStride == dstStride)
    {
		memcpy(dst, src, srcStride * height);
    }
    else
    {
        for (uint32_t i = 0; i < height; i++)
        {
			memcpy(dst + i * dstStride, src + i * srcStride, width * texelSize);
        }
    }
}

}
