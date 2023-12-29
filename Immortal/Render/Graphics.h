#pragma once

#include "Core.h"
#include "Camera.h"
#include "Texture.h"
#include "Shader.h"
#include "Descriptor.h"
#include "WindowCapture.h"
#include "Math/Vector.h"
#include "Device.h"
#include "AsyncCompute.h"

namespace Immortal
{

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

    static inline const char AssetsPathes[][24] = {
        "Assets/Shaders/glsl/",
        "Assets/Shaders/hlsl/"
    };

    static Ref<Shader> GetShader(const std::string &name);

    static auto *Preset()
    {
        return &This->data;
    }

public:
	Graphics(Device *device);
   
    ~Graphics();

public:
	static void SetDevice(Device *device);

    static Device *GetDevice();

    static void Release();

    static Buffer *CreateBuffer(size_t size, BufferType type, const void *data = nullptr);

    static Texture *CreateTexture(const std::string &filepath);

    static Texture *CreateTexture(Format format, uint32_t width, uint32_t height, const void *data = nullptr);

    static void DiscardTexture(const Ref<Texture> &texture);

    static void SetRenderIndex(uint64_t index);

    static Ref<Pipeline> GetPipeline(const std::string &name);

    static void MemoryCopyImage(uint8_t *dst, uint32_t dstStride, const uint8_t *src, uint32_t srcStride, Format format, uint32_t width, uint32_t height);

    template <class T, class ...Args>
	static void Execute(Args && ...args)
    {
		This->thread.Execute<T>(std::forward<Args>(args)...);
    }

public:
	Device *device;

    AsyncComputeThread thread;

    URef<Queue> queue;

    URef<CommandBuffer> commandBuffer;

    Data data;

    URef<Buffer> stagingBuffer;

    uint32_t index = 0;

    std::unordered_map<uint64_t, std::vector<Ref<Texture>>> discardedTextures;

    std::unordered_map<std::string, Ref<Pipeline>> pipelines; 

    static URef<Graphics> This;
};

}
