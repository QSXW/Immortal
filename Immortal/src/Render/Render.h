#pragma once

#include "ImmortalCore.h"
#include "Camera.h"
#include "OrthographicCamera.h"
#include "RenderContext.h"
#include "Renderer.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

namespace Immortal
{

struct RenderData
{
    std::shared_ptr<ShaderMap>   ShaderLibrary;
    std::shared_ptr<Texture2D>   BlackTexture;
    std::shared_ptr<Texture2D>   TransparentTexture;
    std::shared_ptr<Texture2D>   WhiteTexture;
    std::shared_ptr<VertexArray> FullScreenVertexArray;
};

class Render
{
public:
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

    enum class ShaderName : INT32
    {
        Texture,
        PBR,
        Skybox,
        Renderer2D,
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

public:
    static void INIT(RenderContext *context);

    static void Set(Type type)
    {
        API = type;
    }

    static void OnWindowResize(UINT32 width, UINT32 height)
    {
        handle->OnResize(0, 0, width, height);
    }

    static void EnableDepthTest()
    {
        handle->EnableDepthTest();
    }

    static void DisableDepthTest()
    {
        handle->DisableDepthTest();
    }

    static void SetClearColor(Color &color)
    {
        handle->SetClearColor(color);
    }

    static void Clear(Color &color = Color{ 0.0f })
    {
        handle->SetClearColor(color);
        handle->Clear();
    }

    static void DrawIndexed(std::shared_ptr<VertexArray> &vao, UINT32 indexCount)
    {
        handle->DrawIndexed(vao, indexCount);
    }

    static void Render::Begin(OrthographicCamera &camera)
    {
        scene.viewProjectionMatrix = camera.ViewPorjectionMatrix();
    }

    static void Render::End()
    {
        
    }

    static RenderData *Data()
    {
        return &data;
    }

    static uint32_t CurrentPresentedFrameIndex()
    {
        return handle->Index();
    }

    static void Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const Matrix4& transform = Matrix4{ 1.0f });

    static void Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<Mesh> &mesh, const Matrix4 &transform = Matrix4{ 1.0f });

    static void SwapBuffers()
    {
        handle->SwapBuffers();
    }

    static void PrepareFrame()
    {
        handle->PrepareFrame();
    }

    static const char *Api()
    {
        return Stringify[ncast<int>(API)];
    }

    static const char *GraphicsRenderer()
    {
        return handle->GraphicsRenderer();
    }

    static std::shared_ptr<Shader> CreateShader(const std::string &filepath, Shader::Type type = Shader::Type::Graphics)
    {
        return handle->CreateShader(filepath, type);
    }

    static std::shared_ptr<Texture> CreateTexture(const std::string &filepath)
    {
        return handle->CreateTexture(filepath);
    }

private:
    static std::unique_ptr<Renderer> handle;

    struct Scene
    {
        Matrix4 viewProjectionMatrix;
    };

    static RenderData data;
    static Scene scene;

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
