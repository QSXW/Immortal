#include "Graphics.h"
#include "Render2D.h"
#include "Vision/Image.h"
#include "Framework/Timer.h"
#include "FileSystem/Stream.h"

namespace Immortal
{

URef<Graphics> Graphics::This;

Graphics::Graphics(Device *device) :
    device{device},
    thread{device}
{
	This = this;
	commandBuffer = device->CreateCommandBuffer(QueueType::Compute);
}

void Graphics::SetDevice(Device *device)
{
	if (!This)
	{
		new Graphics{device};
	}
}

Device *Graphics::GetDevice()
{
	return This->device;
}

void Graphics::ConstructGlobalVariables()
{
	constexpr uint32_t white = 0xffffffff;
	constexpr uint32_t black = 0x000000ff;
	constexpr uint32_t transparency = 0x00000000;
	constexpr uint32_t normal = 0xffff7f7f;

	This->data.Textures.White = Graphics::CreateTexture(Format::RGBA8, 1, 1, 1, &white);
	This->data.Textures.Black = Graphics::CreateTexture(Format::RGBA8, 1, 1, 1, &black);
	This->data.Textures.Transparent = Graphics::CreateTexture(Format::RGBA8, 1, 1, 1, &transparency);
	This->data.Textures.Normal = Graphics::CreateTexture(Format::RGBA8, 1, 1, 1, &normal);
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
	thread.Join();
	discardedTextures.clear();
	stagingBuffers = {};
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

Texture *Graphics::CreateTexture(const std::string &filepath, AsyncComputeThread *asyncComputeThread)
{
	Picture picture = Vision::Read(filepath);
	if (!picture)
	{
		return nullptr;
	}

	return CreateTexture(picture, asyncComputeThread);
}

Texture *Graphics::CreateTexture(const Picture &picture, AsyncComputeThread *asyncComputeThread)
{
	return CreateTexture(picture.GetFormat(), picture.GetWidth(), picture.GetHeight(), picture.GetStride(0), picture.GetData(), asyncComputeThread);
}

Texture *Graphics::CreateTexture(Format format, uint32_t width, uint32_t height, uint32_t stride, const void *data, AsyncComputeThread *asyncComputeThread)
{
	uint32_t mipLevels = Texture::CalculateMipmapLevels(width, height);
	Texture *texture = This->device->CreateTexture(format, width, height, mipLevels, 1, TextureType::TransferDestination);

    if (!data)
    {
		return texture;
    }

    uint32_t uploadPitch = SLALIGN(stride, TextureAlignment);
	uint32_t uploadSize = height * uploadPitch;

    Ref<Buffer> buffer = {};
	if (uploadSize <= 1024 * 1024)
    {
		std::lock_guard lock{ This->mutex };
		if (!This->stagingBuffers.empty())
		{
			buffer = This->stagingBuffers.front();
			This->stagingBuffers.pop();
		}
    }

	if (!buffer || buffer->GetSize() < uploadSize)
    {
		buffer = This->device->CreateBuffer(uploadSize, BufferType::TransferSource);
    }

    void *mapped = nullptr;
	buffer->Map(&mapped, uploadSize, 0);
	MemoryCopyImage((uint8_t *)mapped, uploadPitch, (uint8_t *)data, stride, format, width, height);
	buffer->Unmap();

	asyncComputeThread->Execute<RecordingTask>([=](uint64_t sync, CommandBuffer *commandBuffer) {
		commandBuffer->CopyBufferToImage(texture, 0, buffer, uploadPitch);
		if (mipLevels > 1)
		{
			commandBuffer->GenerateMipMaps(texture, Filter::Linear);
		}
	});

	asyncComputeThread->Execute<ExecutionCompletedTask>([=]() {
		if (uploadSize <= 1024 * 1024)
		{
			std::lock_guard lock{This->mutex};
			This->stagingBuffers.push(buffer);
		}
	});

    return texture;
}

Shader *Graphics::CreateShader(const std::string &name, ShaderStage stage, const String &path, const std::string &entryPoint)
{
	Stream stream = { path, Stream::Mode::Read };
	if (stream.Readable())
	{
		std::string source;
		stream.Read(source);
		return This->device->CreateShader(name, stage, source, entryPoint);
	}

	return {};
}

void Graphics::DiscardTexture(const Ref<Texture> &texture)
{
    if (texture)
    {
		std::lock_guard lock{ This->discardedMutex };
		This->discardedTextures[This->index].emplace_back(texture);
    }
}

void Graphics::SetRenderIndex(uint64_t index)
{
	std::lock_guard lock{This->discardedMutex};
    if (!This->discardedTextures.empty())
    {
        for (auto it = This->discardedTextures.begin(); it != This->discardedTextures.end();)
        {
			auto &[renderIndex, textures] = *it;

			if (index - renderIndex >= 30)
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
		{ "color_space_yuvp2rgba", { "Assets/Shaders/hlsl/color_space_yuvp2rgba.hlsl", ShaderStage::Compute, "main" } },
	    { "color_space_y2102rgba", { "Assets/Shaders/hlsl/color_space_y2102rgba.hlsl", ShaderStage::Compute, "main" } },
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
