#include "Graphics.h"
#include "Render2D.h"
#include "Vision/Image.h"
#include "Framework/Timer.h"
#include "FileSystem/Stream.h"

namespace Immortal
{

URef<Graphics> Graphics::This;

Graphics::Graphics(Instance *instance, Device *device) :
    instance{instance},
    device{device},
    thread{device},
    releaseThread{1}
{
    thread.SetDescription("Graphics::AsyncComputeThread");
    This = this;
    commandBuffer = device->CreateCommandBuffer(QueueType::Compute);
}

void Graphics::SetDevice(Instance *instance, Device *device)
{
    if (!This)
    {
        new Graphics{instance, device};
    }
}

Instance *Graphics::GetInstance()
{
    return This->instance;
}

Device *Graphics::GetDevice()
{
    return This->device;
}

const FileSystem::Path &Graphics::GetShaderAssetPath()
{
    static const FileSystem::Path &kShaderAssetPath = "Assets/Shaders/hlsl";
    return kShaderAssetPath;
}

void Graphics::ConstructGlobalVariables()
{
    constexpr uint32_t white = 0xffffffff;
    constexpr uint32_t black = 0xff000000;
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
    expiredRenderTargets.clear();
	expiredTextures.clear();
	expiredBuffers.clear();
    stagingBuffers = {};
    device = nullptr;
}

Buffer *Graphics::CreateBuffer(BufferType type, size_t size, const void *data)
{
    auto device = This->device;
	Buffer *buffer = device->CreateBuffer(type, size);

    if (data)
    {
		buffer->Fill(data, size, 0);
    }

    return buffer;
}

Buffer *Graphics::CreateBuffer(BufferType type, size_t size, MemoryType memoryType, Format format)
{
	auto device = Graphics::GetDevice();
	Buffer *buffer = device->CreateBuffer(type, size, memoryType, format);

    return buffer;
}

Ref<Texture> Graphics::CreateTexture(const String &filepath, AsyncComputeThread *asyncComputeThread)
{
    Picture picture = Vision::Read(filepath);
    if (!picture)
    {
        return nullptr;
    }

    return CreateTexture(picture, asyncComputeThread);
}

Ref<Texture> Graphics::CreateTexture(const Picture &picture, AsyncComputeThread *asyncComputeThread)
{
	auto &format = picture.GetFormat();
	auto &width  = picture.GetWidth();
	auto &height = picture.GetHeight();
    if (!asyncComputeThread)
    {
		uint32_t mipLevels = Texture::CalculateMipmapLevels(width, height);
		return Graphics::GetDevice()->CreateTexture(format, width, height, mipLevels, 1, TextureType::TransferDestination | TextureType::Storage);
    }
	return CreateTexture(format, width, height, picture.GetStride(0), picture.GetData(), asyncComputeThread);
}

Ref<Texture> Graphics::CreateTexture(Format format, uint32_t width, uint32_t height, uint32_t stride, const void *data, AsyncComputeThread *asyncComputeThread, uint32_t mipLevels)
{
    if (!mipLevels)
    {
        mipLevels = Texture::CalculateMipmapLevels(width, height);
    }
    Ref<Texture> texture = This->device->CreateTexture(format, width, height, mipLevels, 1, TextureType::TransferDestination | TextureType::Storage);

    if (!data)
    {
        return texture;
    }

    uint32_t uploadPitch = SLALIGN(stride, TextureAlignment);
    uint32_t uploadSize = height * uploadPitch;
    Ref<Buffer> buffer = GetCachedBuffer(BufferType::TransferSource, uploadSize, This->stagingBuffers);
    MemoryCopyImage(buffer, uploadPitch, (uint8_t *) data, stride, format, width, height);

    asyncComputeThread->Execute<RecordingTask>([=](uint64_t sync, CommandBuffer *commandBuffer) {
        commandBuffer->CopyBufferToImage(texture, 0, buffer, uploadPitch);
        if (mipLevels > 1)
        {
            commandBuffer->GenerateMipMaps(texture, Filter::Linear);
        }
    });

    asyncComputeThread->Execute<ExecutionCompletedTask>([buffer]() {
		ReleaseCachedBuffer(BufferType::TransferDestination, buffer);
    });

    return texture;
}

Ref<Buffer> Graphics::GetCachedBuffer(BufferType bufferType, size_t size, std::set<Ref<Buffer>, BufferCompare> &stagingBuffers)
{
    Ref<Buffer> buffer = {};
    if (size <= 1024 * 1024)
    {
        std::lock_guard lock{This->mutex};
        if (!stagingBuffers.empty())
        {
            auto it = stagingBuffers.begin();
            buffer = *it;
            stagingBuffers.erase(it);
        }
    }

    if (!buffer || buffer->GetSize() < size)
    {
		buffer = This->device->CreateBuffer(bufferType, size);
    }

    return buffer;
}

Ref<Buffer> Graphics::GetCachedBuffer(BufferType bufferType, size_t size)
{
	return This->GetCachedBuffer(bufferType, size, bufferType == BufferType::TransferSource ? This->stagingBuffers : This->readBackBuffers);
}

void Graphics::ReleaseCachedBuffer(BufferType bufferType, const Ref<Buffer> &buffer)
{
	if (buffer->GetSize() > 1024 * 1024)
    {
		return;
    }

	std::lock_guard lock{This->mutex};
	if (bufferType == BufferType::TransferSource)
	{
		This->stagingBuffers.insert(buffer);
	}
	else
	{
		This->readBackBuffers.insert(buffer);
	}
}

Picture Graphics::Transfer(const Ref<Texture> &texture, AsyncComputeThread *asyncComputeThread)
{
    auto width  = texture->GetWidth();
    auto heigth = texture->GetHeight();
    auto format = texture->GetFormat();
    Picture picture = Picture{ width, heigth, format, true };
    
    SamplingFactor factors[4];
    GetSamplingFactor(format, factors);

    Format kFormatsNV12[] = { Format::R8_UNORM, Format::R8G8_UNORM };
    Format kFormatsP016[] = { Format::R16_UNORM, Format::R16G16_UNORM};
    for (size_t i = 0; picture.GetData(i); i++)
    {
        Format subformat = format;
        if (format == Format::NV12)
        {
            subformat = kFormatsNV12[i];
        }
        else if (format == Format::P010 || format == Format::P012 || format == Format::P016)
        {
            subformat = kFormatsP016[i];
        }

        uint32_t stride = picture.GetStride(0);
        size_t size = stride * heigth;
        Ref<Buffer> buffer = GetCachedBuffer(BufferType::TransferDestination, size, This->readBackBuffers);
        asyncComputeThread->Execute<RecordingTask>([=](uint64_t sync, CommandBuffer *commandBuffer) {
            commandBuffer->CopyImageToBuffer(buffer, texture, i, stride);
        });

        asyncComputeThread->Execute<ExecutionCompletedTask>([i, stride, factors, subformat, picture, texture, size, buffer]() {
            uint8_t *mapped = nullptr;
            buffer->Map((void **)&mapped, size, 0);
            MemoryCopyImage(picture.GetData(i), picture.GetStride(i), mapped, stride, subformat, picture.GetWidth() >> factors[i].x, picture.GetHeight() >> factors[i].y);
            buffer->Unmap();
            ReleaseCachedBuffer(BufferType::TransferDestination, buffer);
        });
    }

    return picture;
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

template <class T>
void ExpireResource(std::mutex &mutex, uint64_t index, std::unordered_map<uint64_t, std::vector<Ref<T>>> &expiredResources, const Ref<T> &resource, uint64_t offset)
{
	if (resource)
	{
		std::lock_guard lock{mutex};
		expiredResources[index + offset].emplace_back(resource);
	}
}

void Graphics::ReleaseResource(const Ref<RenderTarget> &renderTarget, uint64_t offset)
{
	ExpireResource(This->discardedMutex, This->index, This->expiredRenderTargets, renderTarget, offset);
}

void Graphics::ReleaseResource(const Ref<Texture> &texture, uint64_t offset)
{
	ExpireResource(This->discardedMutex, This->index, This->expiredTextures, texture, offset);
}

void Graphics::ReleaseResource(const Ref<Buffer> &buffer, uint64_t offset)
{
	ExpireResource(This->discardedMutex, This->index, This->expiredBuffers, buffer, offset);
}

template <class T>
void ReleaseResources(ThreadPool &thread, std::unordered_map<uint64_t, std::vector<Ref<T>>> &expiredResources, uint64_t index)
{
	if (!expiredResources.empty())
    {
		for (auto it = expiredResources.begin(); it != expiredResources.end();)
        {
			auto &[renderIndex, resources] = *it;
			if (index - renderIndex >= 3)
			{
				auto r = std::move(*it);
				it = expiredResources.erase(it);
				thread.Enqueue([r] {});
				break;
			}
			else
			{
				it++;
			}
        }
    }
}

void Graphics::SetRenderIndex(GPUEvent *gpuEvent, uint64_t index)
{
	This->gpuEvent  = gpuEvent;
	This->syncValue = index;
    std::lock_guard lock{This->discardedMutex};

    ReleaseResources(This->releaseThread, This->expiredTextures,      index);
	ReleaseResources(This->releaseThread, This->expiredBuffers,       index);
	ReleaseResources(This->releaseThread, This->expiredRenderTargets, index);

    This->index = index;
}

void Graphics::SetSyncEvent(Ref<Texture> &texture)
{
	texture->SetEvent(This->gpuEvent, This->syncValue);
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
	    { "equirect2cube",         { "Assets/Shaders/hlsl/equirect2cube.hlsl",         ShaderStage::Compute, "main" } },
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

void Graphics::MemoryCopyImage(Ref<Buffer> &buffer, uint32_t dstStride, const uint8_t *src, uint32_t srcStride, Format format, uint32_t width, uint32_t height)
{
	void *mapped = {};
	buffer->Map(&mapped, buffer->GetSize(), 0);
    if (mapped)
    {
		MemoryCopyImage((uint8_t *) mapped, dstStride, src, srcStride, format, width, height);
		buffer->Unmap();
		return;
    }

    LOG::ERR("Failed to copy image data to buffer");
}

std::string Graphics::ReadShaderSource(const String &filepath)
{
    Stream stream{ filepath, StreamMode::Read };
    if (!stream.Readable())
    {
        LOG::ERR("Failed to open shader source - `{}`", stream.GetFilePath());
        return {};
    }

    std::string source;
    stream.Read(source);
    
    return source;
}

}
