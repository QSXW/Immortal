#pragma once

#include "Core.h"
#include "Camera.h"
#include "OrthographicCamera.h"
#include "RenderContext.h"
#include "Renderer.h"
#include "Texture.h"
#include "Shader.h"
#include "Descriptor.h"
#include "Math/Vector.h"

namespace Immortal
{

class Render
{
public:
    struct Scene
    {
        Matrix4 viewProjectionMatrix;
    };

    struct Data
    {
        std::shared_ptr<RenderTarget> Target;
        struct {
            std::shared_ptr<Texture> Black;
            std::shared_ptr<Texture> Transparent;
            std::shared_ptr<Texture> White;
        } Textures;
    };

    enum class Type
    {
        None   = 0,
        Vulkan = BIT(1),
        OpenGL = BIT(2),
        D3D12  = BIT(3),
        Metal  = BIT(4)
    };

    static inline const char SAPIs[5][8]
    {
        { "None"   },
        { "Vulkan" },
        { "OpenGL" },
        { "D3D12"  },
        { "Metal"  }
    };

    static inline const char AssetsPathes[][24] = {
        "Assets/Shaders/glsl/",
        "Assets/Shaders/hlsl/"
    };

    static const Shader::Properties ShaderProperties[];

    static Shader::Manager ShaderManager;

    static std::shared_ptr<Shader> GetShader(const std::string &name)
    {
        auto &it = ShaderManager.find(name);
        if (it != ShaderManager.end())
        {
            return it->second;
        }

        LOG::WARN("Request shader {0} not found!", name);
        return nullptr;
    }

    static auto *Preset()
    {
        return &data;
    }

public:
    static void Setup(RenderContext *context);

    static void Setup(const std::shared_ptr<RenderTarget> &renderTarget);

    static const char *Sringify(Render::Type type)
    {
        switch (type)
        {
        case Type::None:   return SAPIs[0];
        case Type::Vulkan: return SAPIs[1];
        case Type::OpenGL: return SAPIs[2];
        case Type::D3D12:  return SAPIs[3];
        case Type::Metal:  return SAPIs[4];
        default:           return SAPIs[0];
        }
    }

    static void Set(Type type)
    {
        API  = type;
        SAPI = Sringify(type);
    }

    static void OnWindowResize(UINT32 width, UINT32 height)
    {
        renderer->OnResize(0, 0, width, height);
    }

    static void SetClearColor(Color &color)
    {
        renderer->SetClearColor(color);
    }

    static void Clear(Color &color = Color{ 0.0f })
    {
        renderer->SetClearColor(color);
        renderer->Clear();
    }

    static void Begin(const Camera &camera)
    {
        scene.viewProjectionMatrix = camera.ViewProjection();
        renderer->Begin(data.Target);
    }

    static void Begin(std::shared_ptr<RenderTarget> &renderTarget)
    {
        user.renderTarget = renderTarget;
        renderer->Begin(user.renderTarget);
    }

    static void Begin(std::shared_ptr<RenderTarget> &renderTarget, const Camera &camera)
    {
        scene.viewProjectionMatrix = camera.ViewProjection();
        user.renderTarget = renderTarget;

        renderer->Begin(user.renderTarget);
    }

    static void End()
    {
        renderer->End();
    }

    static uint32_t CurrentPresentedFrameIndex()
    {
        return renderer->Index();
    }

    static void SwapBuffers()
    {
        renderer->SwapBuffers();
    }

    static void PrepareFrame()
    {
        renderer->PrepareFrame();
    }

    static const char *Api()
    {
        return SAPI;
    }

    static const char *GraphicsRenderer()
    {
        return renderer->GraphicsRenderer();
    }

    static Shader *CreateShader(const std::string &filepath, Shader::Type type = Shader::Type::Graphics)
    {
        return renderer->CreateShader(filepath, type);
    }

    template <class T>
    static Buffer *CreateBuffer(const size_t size, const T *data, Buffer::Type type)
    {
        return renderer->CreateBuffer(size, data, type);
    }

    template <class T>
    static Buffer *CreateBuffer(const std::vector<T> &v, Buffer::Type type)
    {
        return renderer->CreateBuffer(sizeof(T) * v.size(), v.data(), type);
    }

    static Buffer *CreateBuffer(const size_t size, Buffer::Type type)
    {
        return renderer->CreateBuffer(size, type);
    }

    static Buffer *CreateBuffer(const size_t size, uint32_t binding = 0)
    {
        return renderer->CreateBuffer(size, binding);
    }

    static RenderTarget *CreateRenderTarget(const RenderTarget::Description &description)
    {
        return renderer->CreateRenderTarget(description);
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
            return renderer->CreateTexture(std::forward<Args>(args)...);
        }
        if constexpr (IsPrimitiveOf<GraphicsPipeline, T>())
        {
            return renderer->CreateGraphicsPipeline(std::forward<Args>(args)...);
        }
        if constexpr (IsPrimitiveOf<ComputePipeline, T>())
        {
            return renderer->CreateComputePipeline(std::forward<Args>(args)...);
        }
        throw RuntimeException("Type not supported yet");
        return nullptr;
    }

    template <class T>
    static Descriptor *CreateDescriptor(uint32_t count)
    {
        if constexpr (IsPrimitiveOf<Buffer, T>())
        {
            return renderer->CreateBufferDescriptor(count);
        }
        if constexpr (IsPrimitiveOf<Texture, T>())
        {
            return renderer->CreateImageDescriptor(count);
        }
    }

    static void PushConstant(GraphicsPipeline *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset = 0)
    {
        renderer->PushConstant(pipeline, stage, size, data, offset);
    }

    static void PushConstant(ComputePipeline *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset = 0)
    {
        renderer->PushConstant(pipeline, stage, size, data, offset);
    }

    static void Draw(const std::shared_ptr<GraphicsPipeline> &pipeline)
    {
        renderer->Draw(pipeline.get());
    }

    static void Dispatch(ComputePipeline *pipeline, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
    {
        renderer->Dispatch(pipeline, nGroupX, nGroupY, nGroupZ);
    }

private:
    static std::unique_ptr<Renderer> renderer;

    static inline struct
    {
        std::shared_ptr<RenderTarget> renderTarget;
    } user;

    static Data data;

    static Scene scene;

    static inline Vector2 viewport{ 1, 1 };

public:
    static inline Type API{ Type::None };

    static inline const char *SAPI{ nullptr };
};

SL_DEFINE_BITWISE_OPERATION(Render::Type, uint32_t)

template <class SuperType, class OPENGL, class VULKAN, class D3D12, class ... Args>
inline constexpr std::shared_ptr<SuperType> CreateSuper(Args&& ... args)
{
    if (Render::API == Render::Type::OpenGL)
    {
        return std::make_shared<OPENGL>(std::forward<Args>(args)...);
    }
    if (Render::API == Render::Type::Vulkan)
    {
        return std::make_shared<VULKAN>(std::forward<Args>(args)...);
    }
    if (Render::API == Render::Type::D3D12)
    {
        return std::make_shared<D3D12>(std::forward<Args>(args)...);
    }

    SLASSERT(false && "Render API is currently not supported! or Forgot to set the Graphics API!");
    return nullptr;
}

}
