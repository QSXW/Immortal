#pragma once

#include "ImmortalCore.h"
#include "Camera.h"
#include "OrthographicCamera.h"
#include "RenderContext.h"
#include "Renderer.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

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
        std::shared_ptr<Pipeline>     FullScreenPipeline;
        std::shared_ptr<Texture>      BlackTexture;
        std::shared_ptr<Texture>      TransparentTexture;
        std::shared_ptr<Texture>      WhiteTexture;
    };

    enum class Type
    {
        None,
        Vulkan,
        OpenGL,
        D3D12
    };

    static inline const char Stringify[4][8]
    {
        { "None"   },
        { "Vulkan" },
        { "OpenGL" },
        { "D3D12"  }
    };

    static inline const char AssetsPathes[][24] = {
        "Assets/Shaders/glsl/",
        "Assets/Shaders/hlsl/"
    };

    enum class ShaderName : INT32
    {
        Texture,
        Render2D,
        PBR,
        Skybox,
        Tonemap,
        Test,
        Last
    };

    static std::vector<std::shared_ptr<Immortal::Shader>> ShaderContainer;

    static const std::string Render::ShaderProfiles[];

    template <class T, ShaderName U>
    static constexpr inline std::shared_ptr<Shader> Get()
    {
        static_assert(sl::is_same<T, Shader>() && "No suitable Type for getter");
        if constexpr (sl::is_same<T, Shader>())
        {
            static_assert(U < ShaderName::Last, "Shader Index out of bound.");
            return ShaderContainer[ncast<INT32>(U)];
        }
    }

    template <class T>
    static inline std::shared_ptr<Shader> Get(const ShaderName index)
    {
        if constexpr (sl::is_same<T, Shader>())
        {
            size_t i = static_cast<size_t>(index);
            SLASSERT(i < ShaderContainer.size() && "Shader Index out of bound.");
            return ShaderContainer[i];
        }
        static_assert(false, "No suitable Type for getter");
    }

    static auto *Preset()
    {
        return &data;
    }

public:
    static void INIT(RenderContext *context);

    static void Set(Type type)
    {
        API = type;
    }

    static void OnWindowResize(UINT32 width, UINT32 height)
    {
        renderer->OnResize(0, 0, width, height);
    }

    static void EnableDepthTest()
    {
        renderer->EnableDepthTest();
    }

    static void DisableDepthTest()
    {
        renderer->DisableDepthTest();
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

    static void Render::Begin(const Camera &camera)
    {
        scene.viewProjectionMatrix = camera.ViewProjection();
        renderer->Begin(data.Target);
    }

    static void Render::Begin(std::shared_ptr<RenderTarget> &renderTarget, const Camera &camera)
    {
        scene.viewProjectionMatrix = camera.ViewProjection();
        user.renderTarget = renderTarget;

        renderer->Begin(user.renderTarget);
    }

    static void Render::End()
    {
        renderer->End();
    }

    static uint32_t CurrentPresentedFrameIndex()
    {
        return renderer->Index();
    }

    static void Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<Mesh> &mesh, const Matrix4 &transform = Matrix4{ 1.0f });

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
        return Stringify[ncast<int>(API)];
    }

    static const char *GraphicsRenderer()
    {
        return renderer->GraphicsRenderer();
    }

    static std::shared_ptr<Shader> CreateShader(const std::string &filepath, Shader::Type type = Shader::Type::Graphics)
    {
        return renderer->CreateShader(filepath, type);
    }

    static std::shared_ptr<Pipeline> CreatePipeline(std::shared_ptr<Shader> &shader)
    {
        return renderer->CreatePipeline(shader);
    }

    template <class T>
    static std::shared_ptr<Buffer> CreateBuffer(const size_t size, const T *data, Buffer::Type type)
    {
        return renderer->CreateBuffer(sizeof(T) * size, data, type);
    }

    template <class T>
    static std::shared_ptr<Buffer> CreateBuffer(const size_t size, Buffer::Type type)
    {
        return renderer->CreateBuffer(sizeof(T) * size, type);
    }

    static std::shared_ptr<Buffer> CreateBuffer(const size_t size, int binding = 0)
    {
        return renderer->CreateBuffer(size, binding);
    }

    static std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTarget::Description &description)
    {
        return renderer->CreateRenderTarget(description);
    }

    template <class T, class... Args>
    static std::shared_ptr<T> Create(Args&& ... args)
    {
        if constexpr (is_same<T, Buffer>())
        {
            return CreateBuffer(std::forward<Args>(args)...);
        }
        if constexpr (is_same<T, Shader>())
        {
            return CreateShader(std::forward<Args>(args)...);
        }
        if constexpr (is_same<T, RenderTarget>())
        {
            return CreateRenderTarget(std::forward<Args>(args)...);
        }
        if constexpr (is_same<T, Texture>())
        {
            return renderer->CreateTexture(std::forward<Args>(args)...);
        }
        if constexpr (is_same<T, Pipeline>())
        {
            return renderer->CreatePipeline(std::forward<Args>(args)...);
        }
        throw RuntimeException("Type not supported yet");
        return nullptr;
    }

    static void Draw(const std::shared_ptr<Pipeline> &pipeline)
    {
        renderer->Draw(pipeline);
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
};

using ShaderName = Render::ShaderName;

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
