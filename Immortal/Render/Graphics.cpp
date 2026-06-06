#include "Graphics.h"
#include "Render2D.h"
#include "Vision/Image.h"
#include "Framework/Timer.h"
#include "FileSystem/Stream.h"

#include <filesystem>

namespace Immortal
{

URef<Graphics> Graphics::This;

static void DestroyObject(const DeferredObject &object)
{
    switch (object.Type)
    {
    case ObjectType::AccelerationStructure:
        delete static_cast<AccelerationStructure *>(object.Pointer);
        break;
    case ObjectType::RenderTarget:
        delete static_cast<RenderTarget *>(object.Pointer);
        break;
    case ObjectType::Texture:
        delete static_cast<Texture *>(object.Pointer);
        break;
    case ObjectType::Buffer:
        delete static_cast<Buffer *>(object.Pointer);
        break;
    case ObjectType::BufferView:
        delete static_cast<BufferView *>(object.Pointer);
        break;
    case ObjectType::CommandBuffer:
        delete static_cast<CommandBuffer *>(object.Pointer);
        break;
    case ObjectType::DescriptorSet:
        delete static_cast<DescriptorSet *>(object.Pointer);
        break;
    case ObjectType::GPUEvent:
        delete static_cast<GPUEvent *>(object.Pointer);
        break;
    case ObjectType::Pipeline:
        delete static_cast<Pipeline *>(object.Pointer);
        break;
    case ObjectType::PipelineCache:
        delete static_cast<PipelineCache *>(object.Pointer);
        break;
    case ObjectType::Queue:
        delete static_cast<Queue *>(object.Pointer);
        break;
    case ObjectType::Sampler:
        delete static_cast<Sampler *>(object.Pointer);
        break;
    case ObjectType::Shader:
        delete static_cast<Shader *>(object.Pointer);
        break;
    case ObjectType::Swapchain:
        delete static_cast<Swapchain *>(object.Pointer);
        break;
    case ObjectType::Window:
        delete static_cast<Window *>(object.Pointer);
        break;
    case ObjectType::WindowCapture:
        delete static_cast<WindowCapture *>(object.Pointer);
        break;
    default:
        break;
    }
}

static void DestroyObjects(std::vector<DeferredObject> &objects)
{
    for (auto &object : objects)
    {
        DestroyObject(object);
    }
    objects.clear();
}

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
    releaseThread.Join();

    for (auto &[index, objects] : expiredObjects)
    {
        DestroyObjects(objects);
    }
    expiredObjects.clear();

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
    if (format == Format::None)
	{
		LOG_ERROR("Invalid image format specified!");
		return nullptr;
    }

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

    asyncComputeThread->Execute<RecordingTask>([=](CommandBuffer *commandBuffer) {
        commandBuffer->CopyBufferToImage(texture, 0, buffer, uploadPitch);
        if (mipLevels > 1)
        {
            commandBuffer->GenerateMipMaps(texture, Filter::Linear);
        }
    });

    asyncComputeThread->Execute<ExecutionCompletedTask>([buffer]() {
		ReleaseCachedBuffer(BufferType::TransferSource, buffer);
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
		if (This->stagingBuffers.size() >= 3)
		{
			This->stagingBuffers.erase(--This->stagingBuffers.end());
		}
		This->stagingBuffers.insert(buffer);
	}
	else
	{
		if (This->readBackBuffers.size() >= 3)
		{
			This->readBackBuffers.erase(--This->readBackBuffers.end());
		}
		This->readBackBuffers.insert(buffer);
	}
}

static const Format kFormatsNV12[] = {Format::R8_UNORM, Format::R8G8_UNORM};
static const Format kFormatsP016[] = {Format::R16_UNORM, Format::R16G16_UNORM};

Picture Graphics::Transfer(const Ref<Texture> &texture, AsyncComputeThread *asyncComputeThread)
{
    auto width  = texture->GetWidth();
    auto heigth = texture->GetHeight();
	auto format = texture->GetFormat();

    Picture picture = Picture{ width, heigth, format, true };
    
    SamplingFactor factors[4];
    GetSamplingFactor(format, factors);

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
        asyncComputeThread->Execute<RecordingTask>([=](CommandBuffer *commandBuffer) {
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

void Graphics::Transfer(Picture &picture, const std::vector<Ref<Texture>> &textures, AsyncComputeThread *asyncComputeThread)
{
    struct TransferData
    {
		uint32_t width;
		uint32_t height;
		uint32_t stride;
		uint32_t offset;
        Format   format;
		uint32_t subresource;
		uint32_t index;
    };

    TransferData data[4] = {};

    std::vector<Ref<Buffer>> bufs;
	bufs.resize(textures.size());

    SamplingFactor factors[4];
	GetSamplingFactor(picture.GetFormat(), factors);

    bool isPlanar = false;

    size_t size = textures.size();
	if (size == 1)
    {
		auto &format = textures[0]->GetFormat();
		isPlanar = format == Format::NV12 || format == Format::P010 ||
                   format == Format::P012 || format == Format::P016;
		if (isPlanar)
		{
			size = 2;
		}
    }

	auto &texture = textures[0];
	for (size_t i = 0; i < size; i++)
	{
		auto &[width, height, stride, offset, format, subresource, index] = data[i];

		offset = size;
		width  = texture->GetWidth();
		height = texture->GetHeight();
		if (!picture.GetFormat().IsType(Format::YUYV))
		{
			width  >>= factors[i].x;
			height >>= factors[i].y;
		}
            
        if (isPlanar)
		{
			format = texture->GetFormat();
			format = texture->GetFormat() == Format::NV12 ? kFormatsNV12[i] : kFormatsP016[i];
			subresource = i;
		}
        else
		{
			format = textures[i]->GetFormat();
			index = i;
        }
		stride = SLALIGN(width * format.GetTexelSize(), TextureAlignment);

		size_t size = stride * height;
		bufs[i] = GetCachedBuffer(BufferType::TransferDestination, stride * height);
	}

	asyncComputeThread->Execute<RecordingTask>([=](CommandBuffer *commandBuffer) {
        for (size_t i = 0; i < size; i++)
		{
			commandBuffer->CopyImageToBuffer(bufs[i], textures[data[i].index], data[i].subresource, data[i].stride);
        }
	});

   asyncComputeThread->Execute<ExecutionCompletedTask>([=]() {
		for (size_t i = 0; i < size; i++)
		{
			auto &buf = bufs[i];
			uint8_t *mapped = nullptr;
			buf->Map((void **)&mapped, size, 0);
			MemoryCopyImage(picture.GetData(i), picture.GetStride(i), mapped, data[i].stride, data[i].format, data[i].width, data[i].height);
			buf->Unmap();
			ReleaseCachedBuffer(BufferType::TransferDestination, buf);
		}
	});
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

void Graphics::ReleaseResource(const Ref<DescriptorSet> &descriptorSet, uint64_t offset)
{
	ExpireResource(This->discardedMutex, This->index, This->expiredDescriptorSets, descriptorSet, offset);
}

void Graphics::ReleaseResource(const Ref<Pipeline> &pipeline, uint64_t offset)
{
	ExpireResource(This->discardedMutex, This->index, This->expiredPipelines, pipeline, offset);
}

void Graphics::Release(void *pointer, ObjectType type, uint64_t offset)
{
    if (!pointer)
    {
        return;
    }

    if (!This)
    {
        DestroyObject({ pointer, type });
        return;
    }

    std::lock_guard lock{ This->discardedMutex };
    This->expiredObjects[This->index + offset].push_back({ pointer, type });
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

template <class T>
void ReleaseResourcesOnCurrentThread(std::unordered_map<uint64_t, std::vector<Ref<T>>> &expiredResources, uint64_t index)
{
	if (!expiredResources.empty())
    {
		for (auto it = expiredResources.begin(); it != expiredResources.end();)
        {
			auto &[renderIndex, resources] = *it;
			if (index - renderIndex >= 3)
			{
				std::vector<Ref<T>> releasedResources = std::move(resources);
				it = expiredResources.erase(it);
                releasedResources.clear();
				break;
			}
			else
			{
				it++;
			}
        }
    }
}

void ReleaseObjects(ThreadPool &thread, std::unordered_map<uint64_t, std::vector<DeferredObject>> &expiredObjects, uint64_t index)
{
	if (!expiredObjects.empty())
    {
		for (auto it = expiredObjects.begin(); it != expiredObjects.end();)
        {
			auto &[renderIndex, objects] = *it;
			if (index - renderIndex >= 3)
			{
				auto releasedObjects = std::move(objects);
				it = expiredObjects.erase(it);
                std::vector<DeferredObject> threadedObjects;
                for (auto &object : releasedObjects)
                {
                    if (object.Type == ObjectType::Texture)
                    {
                        DestroyObject(object);
                    }
                    else
                    {
                        threadedObjects.emplace_back(object);
                    }
                }
                if (!threadedObjects.empty())
                {
				    thread.Enqueue([objects = std::move(threadedObjects)]() mutable {
                        DestroyObjects(objects);
                    });
                }
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

    // ImGui uses Texture* as ImTextureID, so the final texture unref must stay on the render thread.
    ReleaseResourcesOnCurrentThread(This->expiredTextures,             index);
	ReleaseResources(This->releaseThread, This->expiredBuffers,        index);
	ReleaseResources(This->releaseThread, This->expiredRenderTargets,  index);
	ReleaseResources(This->releaseThread, This->expiredDescriptorSets, index);
	ReleaseResources(This->releaseThread, This->expiredPipelines,      index);
	ReleaseObjects(This->releaseThread, This->expiredObjects,          index);

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
	    { "ibl_irradiance",        { "Assets/Shaders/hlsl/ibl_irradiance.hlsl",        ShaderStage::Compute, "main" } },
	    { "ibl_prefilter",         { "Assets/Shaders/hlsl/ibl_prefilter.hlsl",         ShaderStage::Compute, "main" } },
	    { "brdf_lut",              { "Assets/Shaders/hlsl/brdf_lut.hlsl",              ShaderStage::Compute, "main" } },
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

void Graphics::StorePipeline(const std::string &name, const Ref<Pipeline> &pipeline)
{
	This->pipelines[name] = pipeline;
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

Shader *Graphics::CreateShaderFromDXIL(Device *device, const Path &path, ShaderStage stage)
{
	Stream stream{path, Stream::Mode::Read};
	if (!stream.Readable())
	{
		return nullptr;
	}

	std::vector<uint8_t> dxil;
	stream.Read(dxil);
	return device->CreateShader(stage, ShaderBinaryType::DXIL, dxil.data(), (uint32_t)dxil.size());
}

static FileSystem::Path ResolveShaderHlslPath(const std::string &name)
{
	const FileSystem::Path &assetRoot = Graphics::GetShaderAssetPath();
	FileSystem::Path direct = assetRoot / (name + ".hlsl");
	if (std::filesystem::exists(direct))
	{
		return direct;
	}

	if (name.rfind("MeshletTask_", 0) == 0)
	{
		FileSystem::Path p = assetRoot / "MeshletTask.hlsl";
		if (std::filesystem::exists(p))
		{
			return p;
		}
	}

	const auto pos = name.rfind('_');
	if (pos != std::string::npos && pos + 1 < name.size())
	{
		const std::string suffix = name.substr(pos + 1);
		if (suffix == "VS" || suffix == "PS" || suffix == "HS" || suffix == "DS" || suffix == "GS" || suffix == "MS")
		{
			FileSystem::Path stemPath = assetRoot / (name.substr(0, pos) + ".hlsl");
			if (std::filesystem::exists(stemPath))
			{
				return stemPath;
			}
		}
		/* e.g. MeshletTask_PSMainPhong -> MeshletTask.hlsl */
		FileSystem::Path stemPath2 = assetRoot / (name.substr(0, pos) + ".hlsl");
		if (std::filesystem::exists(stemPath2))
		{
			return stemPath2;
		}
	}

	return {};
}

Shader *Graphics::GetShaderByName(const std::string &name, ShaderStage stage, const std::string &entryPoint)
{
	auto *device = Graphics::GetDevice();
	const FileSystem::Path dxilPath = Graphics::GetShaderAssetPath() / (name + ".dxil");
	if (std::filesystem::exists(dxilPath))
	{
		return Graphics::CreateShaderFromDXIL(device, dxilPath, stage);
	}

	FileSystem::Path hlslPath = ResolveShaderHlslPath(name);
	if (hlslPath.empty())
	{
		LOG::ERR("GetShaderByName: no dxil or hlsl for `{}`", name.c_str());
		return nullptr;
	}

	std::string source = Graphics::ReadShaderSource(hlslPath.string());
	if (source.empty())
	{
		return nullptr;
	}

	return device->CreateShader(name, stage, source, entryPoint);
}

Shader *Graphics::CreateShaderByName(const std::string &name, const std::string &entryPoint)
{
	return Graphics::GetShaderByName(name, ShaderStage::Compute, entryPoint);
}

}
