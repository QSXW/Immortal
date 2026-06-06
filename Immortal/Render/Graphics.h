#pragma once

#include "Core.h"
#include "Graphics/LightGraphics.h"
#include "Vision/Picture.h"
#include "FileSystem/FileSystem.h"

#include <set>
#include <unordered_map>

namespace Immortal
{

enum class ObjectType
{
    None,
    AccelerationStructure,
    RenderTarget,
    Texture,
    Buffer,
    BufferView,
    CommandBuffer,
    DescriptorSet,
    GPUEvent,
    Pipeline,
    PipelineCache,
    Queue,
    Sampler,
    Shader,
    Swapchain,
    Window,
    WindowCapture,
};

struct DeferredObject
{
    void *Pointer = nullptr;
    ObjectType Type = ObjectType::None;
};

struct BufferCompare
{
	bool operator()(const Ref<Buffer> &left, const Ref<Buffer> &right) const
	{
		return left->GetSize() > right->GetSize();
	}
};

class Graphics
{
public:
    struct Data
    {
        struct {
            Ref<Texture> Black;
            Ref<Texture> Transparent;
            Ref<Texture> White;
            Ref<Texture> Normal;
        } Textures;
    };

    static inline const char SAPIs[6][8]
    {
        { "None"   },
        { "Vulkan" },
        { "OpenGL" },
        { "D3D11"  },
        { "D3D12"  },
        { "Metal"  }
    };

    static auto *Preset()
    {
        return &This->data;
    }

public:
    Graphics(Instance *instance, Device *device);

    ~Graphics();

public:
    static void SetDevice(Instance *instance, Device *device);

    static void ConstructGlobalVariables();

    static Instance *GetInstance();

    static Device *GetDevice();

    static const FileSystem::Path &GetShaderAssetPath();

    static void Release();

    static Buffer *CreateBuffer(BufferType type, size_t size, const void *data = nullptr);

    static Buffer *CreateBuffer(BufferType type, size_t size, MemoryType memoryType, Format format = Format::None);

    static Ref<Buffer> GetCachedBuffer(BufferType bufferType, size_t size, std::set<Ref<Buffer>, BufferCompare> &stagingBuffers);

    static Ref<Buffer> GetCachedBuffer(BufferType bufferType, size_t size);

    static void ReleaseCachedBuffer(BufferType bufferType, const Ref<Buffer> &buffer);

    static Ref<Texture> CreateTexture(const String &filepath, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

    static Ref<Texture> CreateTexture(const Picture &picture, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

    static Ref<Texture> CreateTexture(Format format, uint32_t width, uint32_t height, uint32_t stride = 0, const void *data = nullptr, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread(), uint32_t mipLevels = 0);

    static Picture Transfer(const Ref<Texture> &texture, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

    static void Transfer(Picture &pictures, const std::vector<Ref<Texture>> &texture, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

    static Shader *CreateShader(const std::string &name, ShaderStage stage, const String &path, const std::string &entryPoint);

    static void ReleaseResource(const Ref<RenderTarget> &renderTarget, uint64_t offset = 0);

    static void ReleaseResource(const Ref<Texture> &texture, uint64_t offset = 0);

    static void ReleaseResource(const Ref<Buffer> &buffer, uint64_t offset = 0);

    static void ReleaseResource(const Ref<DescriptorSet> &descriptorSet, uint64_t offset = 0);

    static void ReleaseResource(const Ref<Pipeline> &pipeline, uint64_t offset = 0);

    static void Release(void *pointer, ObjectType type, uint64_t offset = 0);

    static void SetRenderIndex(GPUEvent *gpuEvent, uint64_t index);

    static void SetSyncEvent(Ref<Texture> &texture);

    static Ref<Pipeline> GetPipeline(const std::string &name);

    static void StorePipeline(const std::string &name, const Ref<Pipeline> &pipeline);

    static void MemoryCopyImage(uint8_t *dst, uint32_t dstStride, const uint8_t *src, uint32_t srcStride, Format format, uint32_t width, uint32_t height);

    static void MemoryCopyImage(Ref<Buffer> &buffer, uint32_t dstStride, const uint8_t *src, uint32_t srcStride, Format format, uint32_t width, uint32_t height);

    static std::string ReadShaderSource(const String &filepath);

    static Shader *CreateShaderFromDXIL(Device *device, const Path &path, ShaderStage stage = ShaderStage::Compute);

    static Shader *GetShaderByName(const std::string &name, ShaderStage stage, const std::string &entryPoint);

    static Shader *CreateShaderByName(const std::string &name, const std::string &entryPoint = "main");

    template <class T, class ...Args>
    static void Execute(Args && ...args)
    {
        This->thread.Execute<T>(std::forward<Args>(args)...);
    }

    static void WaitIdle()
    {
        This->thread.WaitIdle();
    }

    static AsyncComputeThread *GetAsyncComputeThread()
    {
        return &This->thread;
    }

public:
	Instance *instance;

    Device *device;

    AsyncComputeThread thread;

    URef<Queue> queue;

    URef<CommandBuffer> commandBuffer;

    Data data;

    std::mutex mutex;

    std::set<Ref<Buffer>, BufferCompare> stagingBuffers;

    std::set<Ref<Buffer>, BufferCompare> readBackBuffers;

    uint32_t index = 0;

    std::mutex discardedMutex;

    std::unordered_map<uint64_t, std::vector<Ref<RenderTarget>>> expiredRenderTargets;

    std::unordered_map<uint64_t, std::vector<Ref<Texture>>> expiredTextures;

	std::unordered_map<uint64_t, std::vector<Ref<Buffer>>> expiredBuffers;

    std::unordered_map<uint64_t, std::vector<Ref<DescriptorSet>>> expiredDescriptorSets;

    std::unordered_map<uint64_t, std::vector<Ref<Pipeline>>> expiredPipelines;

    std::unordered_map<uint64_t, std::vector<DeferredObject>> expiredObjects;

    std::unordered_map<std::string, Ref<Pipeline>> pipelines;

    ThreadPool releaseThread;

    GPUEvent *gpuEvent;

    uint64_t syncValue;

    static URef<Graphics> This;
};

}
