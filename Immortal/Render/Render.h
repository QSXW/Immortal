#pragma once

#include "Core.h"
#include "Camera.h"
#include "RenderContext.h"
#include "Texture.h"
#include "Shader.h"
#include "Descriptor.h"
#include "WindowCapture.h"
#include "Math/Vector.h"

namespace Immortal
{

struct Pipelines
{

static Ref<ComputePipeline> ColorSpace;

static Ref<ComputePipeline> ColorSpaceNV122RGBA8;

};


class Render
{
public:
    using Pipelines = Immortal::Pipelines;

    struct Data
    {
        Ref<RenderTarget> Target;
        struct {
            Ref<Texture> Black;
            Ref<Texture> Transparent;
            Ref<Texture> White;
            Ref<Texture> Normal;
        } Textures;
    };

    enum class Type
    {
        None   = 0,
        Vulkan = BIT(2),
        OpenGL = BIT(3),
		Metal  = BIT(4),
        D3D    = BIT(5),
		D3D11  = BITS(D3D, BIT(0)),
		D3D12  = BITS(D3D, BIT(1)),
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

    static const Shader::Properties ShaderProperties[];

    static Shader::Manager ShaderManager;

    static Ref<Shader> GetShader(const std::string &name);

    static auto *Preset()
    {
        return &data;
    }

public:
    static void Setup(RenderContext *context);

    static void Release();

    static void Setup(Ref<RenderTarget> &renderTarget);

    static const char *Sringify(Render::Type type)
    {
        switch (type)
        {
        case Type::None:   return SAPIs[0];
        case Type::Vulkan: return SAPIs[1];
        case Type::OpenGL: return SAPIs[2];
		case Type::D3D11:  return SAPIs[3];
        case Type::D3D12:  return SAPIs[4];
        case Type::Metal:  return SAPIs[5];
        default:           return SAPIs[0];
        }
    }

    static void Set(Type type, int id = AUTO_DEVICE_ID)
    {
        API      = type;
        DeviceId = id;
        SAPI     = Sringify(type);
    }

    template <class T>
    requires std::is_same_v<RenderContext, T>
    static T *GetAddress()
    {
		return renderContext;
    }

    static Anonymous GetDevice()
    {
		return renderContext->GetDevice();
    }

    static void OnWindowResize(UINT32 width, UINT32 height)
    {
        renderContext->OnResize(0, 0, width, height);
    }

    static void Begin(Ref<RenderTarget> &renderTarget)
    {
        renderContext->Begin(renderTarget);
    }

    static void Begin(Ref<RenderTarget> &renderTarget, const Camera &camera)
    {
        renderContext->Begin(renderTarget);
    }

    static void End()
    {
        renderContext->End();
    }

    static void SwapBuffers()
    {
        renderContext->SwapBuffers();
    }

    static void PrepareFrame()
    {
        renderContext->PrepareFrame();
    }

    static const char *Api()
    {
        return SAPI;
    }

    static const char *GraphicsRenderer()
    {
        return renderContext->GraphicsRenderer();
    }

    static Shader *CreateShader(const std::string &filepath, Shader::Type type = Shader::Type::Graphics)
    {
        return renderContext->CreateShader(filepath, type);
    }

    template <class T>
    static Buffer *CreateBuffer(const size_t size, const T *data, Buffer::Type type)
    {
        return renderContext->CreateBuffer(size, data, type);
    }

    template <class T>
    static Buffer *CreateBuffer(const std::vector<T> &v, Buffer::Type type)
    {
        return renderContext->CreateBuffer(sizeof(T) * v.size(), v.data(), type);
    }

    static Buffer *CreateBuffer(const size_t size, Buffer::Type type)
    {
        return renderContext->CreateBuffer(size, type);
    }

    static Image *CreateImage(const std::string &filepath, const Texture::Description &desc = {})
    {
        return renderContext->CreateTexture(filepath, desc);
    }

    static Texture *CreateTexture(const std::string &filepath, const Texture::Description &desc = {})
    {
        return renderContext->CreateTexture(filepath, desc);
    }

    static WindowCapture *CreateWindowCapture()
    {
        return renderContext->CreateWindowCapture();
    }

    static Buffer *CreateBuffer(const size_t size, uint32_t binding = 0)
    {
        return renderContext->CreateBuffer(size, binding);
    }

    static RenderTarget *CreateRenderTarget(const RenderTarget::Description &description)
    {
        return renderContext->CreateRenderTarget(description);
    }

    static AccelerationStructure *CreateAccelerationStructure(const Buffer *pVertexBuffer, const InputElementDescription &desc, const Buffer *pIndexBuffer, const Buffer *pTranformBuffer)
    {
        return renderContext->CreateAccelerationStructure(pVertexBuffer, desc, pIndexBuffer, pTranformBuffer);
    }

    template <class T, class... Args>
    static T *Create(Args&& ... args)
    {
        if constexpr (IsPrimitiveOf<Buffer, T>())
        {
            return CreateBuffer(std::forward<Args>(args)...);
        }
        if constexpr (IsPrimitiveOf<Shader, T>())
        {
            return CreateShader(std::forward<Args>(args)...);
        }
        if constexpr (IsPrimitiveOf<RenderTarget, T>())
        {
            return CreateRenderTarget(std::forward<Args>(args)...);
        }
        if constexpr (IsPrimitiveOf<Texture, T>())
        {
            return renderContext->CreateTexture(std::forward<Args>(args)...);
        }
        if constexpr (IsPrimitiveOf<TextureCube, T>())
        {
            return renderContext->CreateTextureCube(std::forward<Args>(args)...);
        }
        if constexpr (IsPrimitiveOf<GraphicsPipeline, T>())
        {
            return renderContext->CreateGraphicsPipeline(std::forward<Args>(args)...);
        }
        if constexpr (IsPrimitiveOf<ComputePipeline, T>())
        {
            return renderContext->CreateComputePipeline(std::forward<Args>(args)...);
        }
        throw RuntimeException("Type not supported yet");
        return nullptr;
    }

    template <class T>
    static DescriptorBuffer *CreateDescriptor(uint32_t count)
    {
        if constexpr (IsPrimitiveOf<Buffer, T>())
        {
            return renderContext->CreateBufferDescriptor(count);
        }
        if constexpr (IsPrimitiveOf<Texture, T>())
        {
            return renderContext->CreateImageDescriptor(count);
        }
    }

    static void PushConstant(GraphicsPipeline *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset = 0)
    {
        renderContext->PushConstant(pipeline, stage, size, data, offset);
    }

    static void PushConstant(ComputePipeline *pipeline, uint32_t size, const void *data, uint32_t offset = 0)
    {
        renderContext->PushConstant(pipeline, size, data, offset);
    }

    static void Draw(GraphicsPipeline *pipeline)
    {
		renderContext->Draw(pipeline);
    }

    static void Dispatch(ComputePipeline *pipeline, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
    {
		renderContext->Dispatch(pipeline, nGroupX, nGroupY, nGroupZ);
    }

    static void Blit(Texture *texture)
    {
		renderContext->Blit(texture);
    }

private:
	static RenderContext *renderContext;

    static Data data;

    static inline Vector2 viewport{ 1, 1 };

public:
    static inline Type API = Type::None;

    static inline int DeviceId = 0;

    static inline const char *SAPI = "";
};

SL_ENABLE_BITWISE_OPERATOR(Render::Type)

}
